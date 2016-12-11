

#include "engine_int.hpp"
#include "engext.hpp"

extern AnimCharHashTable* g_AnimChars;



enum MEOpType
{
	OT_ADD = 0,  // +
	OT_SUB = 1,  // -
	OT_MUL = 2,  // *
	OT_DIV = 3,  // /
	OT_MOD = 4,  // %
	OT_EQ  = 5,  // ==
	OT_NEQ = 6,  // !=
	OT_LT  = 7,  // <
	OT_GT  = 8,  // >
	OT_LTE = 9,  // <=
	OT_GTE = 10, // >=
	OT_AND = 11, // &&
	OT_OR  = 12, // ||
	OT_XOR = 13, // ^
	OT_NOT = 14, // !
	OT_NEG = 15, // -
	OT_POW = 16, // **
	OT_ABS = 17, // ~
	
	OT_NOP = 253, // marked for removal
	OT_VAR = 254, // variable
	OT_VAL = 255, // constant
};
#define ME_OPCODE_TYPE( c ) ((c)&0xff)
#define ME_OPCODE_OP1CONST 0x100
#define ME_OPCODE_OP1VARBL 0x200
#define ME_OPCODE_OP2CONST 0x400
#define ME_OPCODE_OP2VARBL 0x800
enum METokenType
{
	TT_None,
	TT_Error,
	
	TT_Ident,
	TT_Const,
	TT_Operator,
	TT_LParen,
	TT_RParen,
};
struct METoken
{
	METokenType type;
	StringView data;
};

#define _ME_IsWhitespace( ch ) ((ch)==' '||(ch)=='\t'||(ch)=='\n'||(ch)=='\r')
#define _ME_IsIdentStart( ch ) (((ch)>='a'&&(ch)<='z')||((ch)>='A'&&(ch)<='Z')||(ch)=='_')
#define _ME_IsNumeric( ch ) ((ch)>='0'&&(ch)<='9')
#define _ME_IsIdentPart( ch ) (_ME_IsIdentStart(ch)||_ME_IsNumeric(ch))
#define _ME_IsNumberPart( ch ) (_ME_IsNumeric(ch)||(ch)=='.')
#define _ME_OpChars "+-*/%<>=!&|^~"
METoken _ME_GetNextToken( StringView& script )
{
	char ch = script.ch();
	// skip whitespace
	while( _ME_IsWhitespace( ch ) )
	{
		script.skip( 1 );
		ch = script.ch();
	}
	
	if( script.size() == 0 )
	{
		// end of script
		METoken out = { TT_None, SV() };
		return out;
	}
	// identifier
	else if( ch == '(' || ch == ')' )
	{
		METoken out = { ch == '(' ? TT_LParen : TT_RParen, script.take( 1 ) };
		return out;
	}
	// numeric constant
	else if( _ME_IsNumeric( ch ) )
	{
		size_t end = 1;
		while( end < script.size() && _ME_IsNumberPart( script[ end ] ) )
			end++;
		METoken out = { TT_Const, script.take( end ) };
		return out;
	}
	// identifiers/keywords
	else if( _ME_IsIdentStart( ch ) )
	{
		size_t end = 1;
		while( end < script.size() && _ME_IsIdentPart( script[ end ] ) )
			end++;
		METoken out = { TT_Ident, script.take( end ) };
		return out;
	}
	// operators
	else if( strchr( _ME_OpChars, ch ) )
	{
		size_t end = 1;
		if( script.starts_with( "==" ) ||
			script.starts_with( "!=" ) ||
			script.starts_with( "<=" ) ||
			script.starts_with( ">=" ) ||
			script.starts_with( "&&" ) ||
			script.starts_with( "||" ) ||
			script.starts_with( "**" ) )
			end = 2;
		METoken out = { TT_Operator, script.take( end ) };
		return out;
	}
	else
	{
		// invalid character
		METoken out = { TT_Error, SV() };
		return out;
	}
}

static int _ME_TokenScore( const METoken& T, bool unary )
{
	if( T.type == TT_Operator )
	{
		if( unary )
		{
			if( T.data == "+" || T.data == "-" ||
				T.data == "!" || T.data == "~" ) return 20;
		}
		
		if( T.data == "&&" || T.data == "||" || T.data == "^" ) return 90;
		if( T.data == "==" || T.data == "!=" ||
			T.data == "<" || T.data == ">" ||
			T.data == "<=" || T.data == ">=" ) return 70;
		if( T.data == "+" || T.data == "-" ) return 50;
		if( T.data == "*" || T.data == "/" || T.data == "%" ) return 30;
		if( T.data == "**" ) return 25;
	}
	return 10;
}

struct MEPTRes
{
	MEPTRes( const MECompileResult& cr ) : error( cr ){ ASSERT( cr.error ); }
	MEPTRes( uint16_t p ) : at( p ){ ASSERT( p != ME_OPERAND_NONE ); }
	
	MECompileResult error;
	uint16_t at;
};

MEPTRes MathEquation::_AllocOper()
{
	if( ops.size() >= ME_OPERAND_NONE )
		return MECompileResult( "", "expression too complex" );
	MEOperation op = { OT_NOP, 0, 0 };
	ops.push_back( op );
	return uint16_t( ops.size() - 1 );
}

MEPTRes MathEquation::_AllocConst( double val )
{
	for( size_t i = 0; i < consts.size(); ++i )
	{
		if( consts[ i ] == val )
			return uint16_t(i);
	}
	if( consts.size() >= 0xffff )
		return MECompileResult( "", "too many constants" );
	consts.push_back( val );
	return uint16_t( consts.size() - 1 );
}

static uint16_t _ME_Str1Op( StringView str )
{
	if( str == "-" ) return OT_NEG;
	if( str == "!" ) return OT_NOT;
	if( str == "~" ) return OT_ABS;
	return OT_NOP;
}

static uint16_t _ME_Str2Op( StringView str )
{
	if( str == "+" ) return OT_ADD;
	if( str == "-" ) return OT_SUB;
	if( str == "*" ) return OT_MUL;
	if( str == "/" ) return OT_DIV;
	if( str == "%" ) return OT_MOD;
	if( str == "==" ) return OT_EQ;
	if( str == "!=" ) return OT_NEQ;
	if( str == "<" ) return OT_LT;
	if( str == ">" ) return OT_GT;
	if( str == "<=" ) return OT_LTE;
	if( str == ">=" ) return OT_GTE;
	if( str == "&&" ) return OT_AND;
	if( str == "||" ) return OT_OR;
	if( str == "^" ) return OT_XOR;
	if( str == "**" ) return OT_POW;
	return OT_NOP;
}

void MathEquation::_Optimize( MEOperation& O )
{
	// this function cannot increase operation count
	// so a reference argument is acceptable
	int t = ME_OPCODE_TYPE( O.type );
	if( t == OT_NOP || t == OT_VAR || t == OT_VAL )
		return; // nothing to be done here
	
	bool hasop2 = t != OT_NEG && t != OT_NOT && t != OT_ABS;
	// assume operands are already optimized
	// _Optimize( ops[ O.op1 ] );
	// _Optimize( ops[ O.op2 ] );
	
	// integrate operand variable/const accesses into current operation
	if( ME_OPCODE_TYPE( ops[ O.op1 ].type ) == OT_VAR )
	{
		ops[ O.op1 ].type = OT_NOP;
		O.op1 = ops[ O.op1 ].op1;
		O.type |= ME_OPCODE_OP1VARBL;
	}
	else if( ME_OPCODE_TYPE( ops[ O.op1 ].type ) == OT_VAL )
	{
		ops[ O.op1 ].type = OT_NOP;
		O.op1 = ops[ O.op1 ].op1;
		O.type |= ME_OPCODE_OP1CONST;
	}
	
	if( hasop2 )
	{
		if( ME_OPCODE_TYPE( ops[ O.op2 ].type ) == OT_VAR )
		{
			ops[ O.op2 ].type = OT_NOP;
			O.op2 = ops[ O.op2 ].op1;
			O.type |= ME_OPCODE_OP2VARBL;
		}
		else if( ME_OPCODE_TYPE( ops[ O.op2 ].type ) == OT_VAL )
		{
			ops[ O.op2 ].type = OT_NOP;
			O.op2 = ops[ O.op2 ].op1;
			O.type |= ME_OPCODE_OP2CONST;
		}
	}
	
	// if all operands are consts, execute operation
	if( ( O.type & ME_OPCODE_OP1CONST ) &&
		( !hasop2 || ( O.type & ME_OPCODE_OP2CONST ) ) &&
		consts.size() < 0xffff )
	{
		// vars = NULL because operation does not use them
		double val = _DoOp( &O - ops.data(), NULL );
		O.type = OT_VAL;
		O.op1 = _AllocConst( val ).at;
	}
}

MEPTRes MathEquation::_ParseTokens(
	ArrayView< METoken > tokenlist,
	StringView parentfirst,
	const MEVariableInterface* vars
)
{
	if( !tokenlist.size() )
		return MECompileResult( parentfirst, "expected token" );
	
	// constant
	if( tokenlist.size() == 1 && tokenlist[0].type == TT_Const )
	{
		MEPTRes op = _AllocOper();
		if( op.error )
			return op;
		bool suc = false;
		double val = String_ParseFloat( tokenlist[0].data, &suc );
		if( !suc )
			return MECompileResult( tokenlist[0].data, "failed to parse number" );
		MEPTRes cns = _AllocConst( val );
		if( cns.error )
			return cns;
		MEOperation& O = ops[ op.at ];
		O.type = OT_VAL;
		O.op1 = cns.at;
		return op;
	}
	
	// variable
	if( tokenlist.size() == 1 && tokenlist[0].type == TT_Ident )
	{
		MEPTRes op = _AllocOper();
		if( op.error )
			return op;
		MEOperation& O = ops[ op.at ];
		O.type = OT_VAL;
		double value = 0;
		
		StringView name = tokenlist[0].data;
		if( name == "true" ) value = 1;
		else if( name == "false" ) value = 0;
		else if( name == "pi" ) value = M_PI;
		else if( name == "m_deg2rad" ) value = M_PI / 180.0;
		else if( name == "m_rad2deg" ) value = 180.0 / M_PI;
		else
		{
			O.type = OT_VAR;
			O.op1 = vars->MEGetID( tokenlist[0].data );
			if( O.op1 == ME_OPERAND_NONE )
				return MECompileResult( name, "variable not found" );
			_Optimize( O );
		}
		
		if( O.type == OT_VAL )
		{
			MEPTRes cns = _AllocConst( value );
			if( cns.error )
				return cns;
			O.op1 = cns.at;
		}
		
		return op;
	}
	
	// binary operators
	int level = 0;
	size_t best_token_id = NOT_FOUND;
	int best_token_score = 0;
	
	for( size_t i = 0; i < tokenlist.size(); ++i )
	{
		const METoken& T = tokenlist[ i ];
		
		if( T.type == TT_LParen )
		{
			level++;
		}
		else if( T.type == TT_RParen )
		{
			level--;
			if( level < 0 )
			{
				return MECompileResult( T.data, "too many closing parentheses" );
			}
		}
		else if( level > 0 )
			continue; // don't parse inside parentheses
		
		int score = _ME_TokenScore( T, i > 0 && tokenlist[ i - 1 ].type == TT_Operator );
		if( score >= best_token_score )
		{
			best_token_id = i;
			best_token_score = score;
		}
	}
	
	if( level > 0 )
	{
		return MECompileResult( tokenlist[0].data, "unmatched parenthesis" );
	}
	if( best_token_id == NOT_FOUND ||
		best_token_id == 0 ||
		best_token_id == tokenlist.size() - 1 )
	{
		// subexpression
		if( tokenlist.size() >= 2 &&
			tokenlist[0].type == TT_LParen &&
			tokenlist[ tokenlist.size() - 1 ].type == TT_RParen )
			return _ParseTokens( tokenlist.part( 1, tokenlist.size() - 2 ), tokenlist[0].data, vars );
		
		// unary operator
		if( tokenlist[0].type == TT_Operator )
		{
			if( tokenlist[0].data == "+" )
			{
				return _ParseTokens( tokenlist.part( 1 ), tokenlist[0].data, vars );
			}
			else if( tokenlist[0].data == "-" ||
				tokenlist[0].data == "!" ||
				tokenlist[0].data == "~" )
			{
				MEPTRes op = _AllocOper();
				if( op.error )
					return op;
				ops[ op.at ].type = _ME_Str1Op( tokenlist[0].data );
				MEPTRes src = _ParseTokens( tokenlist.part( 1 ), tokenlist[0].data, vars );
				if( src.error )
					return src;
				ops[ op.at ].op1 = src.at;
				
				_Optimize( ops[ op.at ] );
				return op;
			}
			else
			{
				return MECompileResult( tokenlist[0].data, "invalid unary operator" );
			}
		}
		
		return MECompileResult( tokenlist[0].data, "invalid expression" );
	}
	
	MEPTRes op = _AllocOper();
	if( op.error )
		return op;
	ops[ op.at ].type = _ME_Str2Op( tokenlist[ best_token_id ].data );
	if( ops[ op.at ].type == OT_NOP )
		return MECompileResult( tokenlist[ best_token_id ].data, "bad operator" );
	
	MEPTRes src1 = _ParseTokens( tokenlist.part( 0, best_token_id ), tokenlist[ 0 ].data, vars );
	if( src1.error )
		return src1;
	ASSERT( size_t(src1.at) < ops.size() );
	ops[ op.at ].op1 = src1.at;
	
	MEPTRes src2 = _ParseTokens( tokenlist.part( best_token_id + 1 ), tokenlist[ best_token_id + 1 ].data, vars );
	if( src2.error )
		return src2;
	ASSERT( size_t(src2.at) < ops.size() );
	ops[ op.at ].op2 = src2.at;
	
	_Optimize( ops[ op.at ] );
	return op;
}

void MathEquation::_Clean()
{
	// remove unused operations
	for( size_t i = ops.size(); i > 0; )
	{
		--i;
		if( ops[ i ].type == OT_NOP )
		{
			// decrement references from previous ops
			for( size_t j = 0; j < i; ++j )
			{
				MEOperation& op = ops[ j ];
				if( ME_OPCODE_TYPE( op.type ) == OT_VAR ||
					ME_OPCODE_TYPE( op.type ) == OT_VAL )
					continue;
				else if( !( op.type & (ME_OPCODE_OP1CONST|ME_OPCODE_OP1VARBL) ) &&
					op.op1 >= i )
					op.op1--;
				else if( !( op.type & (ME_OPCODE_OP2CONST|ME_OPCODE_OP2VARBL) ) &&
					op.op2 >= i )
					op.op2--;
			}
			// remove operation
			ops.erase( i );
		}
	}
	
	// find unused constants
	Array< bool > usage;
	usage.resize_using( consts.size(), false );
	for( size_t i = 0; i < ops.size(); ++i )
	{
		MEOperation& op = ops[ i ];
		if( ME_OPCODE_TYPE( op.type ) == OT_VAL )
			usage[ op.op1 ] = true;
		else if( op.type & ME_OPCODE_OP1CONST )
			usage[ op.op1 ] = true;
		else if( op.type & ME_OPCODE_OP2CONST )
			usage[ op.op2 ] = true;
	}
	
	// remove unused constants
	for( size_t i = consts.size(); i > 0; )
	{
		--i;
		if( usage[ i ] )
			continue;
		// decrement references in ops
		for( size_t j = 0; j < ops.size(); ++j )
		{
			MEOperation& op = ops[ j ];
			if( ME_OPCODE_TYPE( op.type ) == OT_VAL && op.op1 >= i )
				op.op1--;
			else if( op.type & ME_OPCODE_OP1CONST && op.op1 >= i )
				op.op1--;
			else if( op.type & ME_OPCODE_OP2CONST && op.op2 >= i )
				op.op2--;
		}
		// remove constant
		consts.erase( i );
	}
}

void MathEquation::Clear()
{
	consts.clear();
	ops.clear();
}

MECompileResult MathEquation::Compile( StringView script, const MEVariableInterface* vars )
{
	METoken token;
	Array< METoken > tokenlist;
	StringView origscript = script;
	
	Clear();
	
	// parse tokens
	while( ( token = _ME_GetNextToken( script ) ).type > TT_Error )
		tokenlist.push_back( token );
	if( token.type == TT_Error )
		return MECompileResult( script, "invalid character" );
	
	// recursively split tokens into a tree
	MECompileResult error = _ParseTokens(
		tokenlist,
		tokenlist.size() ? tokenlist[0].data : origscript,
		vars
	).error;
	if( error )
	{
		Clear();
		return error;
	}
	
	_Clean();
	
#if ME_DUMP_COMPILED
	Dump();
#endif
	return MECompileResult();
}

double MathEquation::Eval( const MEVariableInterface* vars ) const
{
	return _DoOp( 0, vars );
}

void MathEquation::Dump() const
{
	printf( "MATH_EQUATION %p\n", this );
	for( size_t i = 0; i < consts.size(); ++i )
		printf( "CONST #%03d: %.16g\n", int(i), consts[ i ] );
	for( size_t i = 0; i < ops.size(); ++i )
	{
		const MEOperation& O = ops[ i ];
		printf( "OP #%03u: T%03uF%x O1:%03u O2:%03u\n",
			int(i), ME_OPCODE_TYPE( O.type ), int((O.type>>8)&0xf),
			int(O.op1), int(O.op2) );
	}
}

double MathEquation::_Op1( const MEOperation& O, const MEVariableInterface* vars ) const
{
	if( O.type & ME_OPCODE_OP1CONST )
	{
		if( (size_t) O.op1 >= consts.size() )
			return 0;
		return consts[ O.op1 ];
	}
	else if( O.type & ME_OPCODE_OP1VARBL )
	{
		return vars->MEGetValue( O.op1 );
	}
	else return _DoOp( O.op1, vars );
}

double MathEquation::_Op2( const MEOperation& O, const MEVariableInterface* vars ) const
{
	if( O.type & ME_OPCODE_OP2CONST )
	{
		if( (size_t) O.op2 >= consts.size() )
			return 0;
		return consts[ O.op2 ];
	}
	else if( O.type & ME_OPCODE_OP2VARBL )
	{
		return vars->MEGetValue( O.op2 );
	}
	else return _DoOp( O.op2, vars );
}

static double safe_dpow( double x, double y )
{
	if( x < 0 && y != int(y) ) return 0;
	if( x == 0 && y <= 0 ) return 0;
	return pow( x, y );
}

double MathEquation::_DoOp( uint16_t op, const MEVariableInterface* vars ) const
{
	if( (size_t) op >= ops.size() )
		return 0.0;
	const MEOperation& O = ops[ op ];
	switch( ME_OPCODE_TYPE( O.type ) )
	{
	case OT_ADD: return _Op1( O, vars ) + _Op2( O, vars );
	case OT_SUB: return _Op1( O, vars ) - _Op2( O, vars );
	case OT_MUL: return _Op1( O, vars ) * _Op2( O, vars );
	case OT_DIV: return safe_ddiv( _Op1( O, vars ), _Op2( O, vars ) );
	case OT_MOD: return safe_dmod( _Op1( O, vars ), _Op2( O, vars ) );
	case OT_EQ:  return _Op1( O, vars ) == _Op2( O, vars );
	case OT_NEQ: return _Op1( O, vars ) != _Op2( O, vars );
	case OT_LT:  return _Op1( O, vars ) < _Op2( O, vars );
	case OT_GT:  return _Op1( O, vars ) > _Op2( O, vars );
	case OT_LTE: return _Op1( O, vars ) <= _Op2( O, vars );
	case OT_GTE: return _Op1( O, vars ) >= _Op2( O, vars );
	case OT_AND: return _Op1( O, vars ) && _Op2( O, vars );
	case OT_OR:  return _Op1( O, vars ) || _Op2( O, vars );
	case OT_XOR: return (!!_Op1( O, vars )) ^ (!!_Op2( O, vars ));
	case OT_NOT: return !_Op1( O, vars );
	case OT_NEG: return -_Op1( O, vars );
	case OT_POW: return safe_dpow( _Op1( O, vars ), _Op2( O, vars ) );
	case OT_ABS: return fabs( _Op1( O, vars ) );
	case OT_VAR: return vars->MEGetValue( O.op1 );
	case OT_VAL:
		if( (size_t) O.op1 >= consts.size() )
			return 0;
		return consts[ O.op1 ];
	default: return 0;
	}
}



//////////////////////
//  R A G D O L L
////////////////////

AnimRagdoll::AnimRagdoll( PhyWorldHandle phyWorld ) :
	m_enabled( false ),
	m_lastTickSize( 0 ),
	m_phyWorld( phyWorld )
{
	ASSERT( phyWorld && "physics world handle must be valid" );
}

void AnimRagdoll::Initialize( AnimCharInst* chinst )
{
	AnimCharacter* chinfo = chinst->animChar;
	SGRX_PhyRigidBodyInfo rbinfo;
	rbinfo.enabled = false;
	rbinfo.friction = 0.8f;
	rbinfo.restitution = 0.02f;
	rbinfo.linearDamping = 0.1f;
	rbinfo.angularDamping = 0.1f;
	rbinfo.group = 4;
	
	for( size_t i = 0; i < m_pose.size(); ++i )
	{
		Body B = { V3(0), Quat::Identity,
			NULL, NULL, V3(0), V3(0), Quat::Identity, Quat::Identity };
		m_bones[ i ] = B;
	}
	
	if( !chinfo || m_pose.size() != chinfo->bones.size() )
	{
		// ragdoll not used
		return;
	}
	
	for( size_t bid = 0; bid < chinfo->bones.size(); ++bid )
	{
		const AnimCharacter::BoneInfo& BI = chinfo->bones[ bid ];
		
		Body* TB = &m_bones[ chinst->m_bone_ids[ bid ] ];
		
	//	LOG << SB.name << " > " << SB.capsule_radius << "|" << SB.capsule_height;
		if( BI.body.type == AnimCharacter::BodyType_Sphere )
		{
			rbinfo.shape = m_phyWorld->CreateSphereShape( BI.body.size.x );
		}
		else if( BI.body.type == AnimCharacter::BodyType_Capsule )
		{
			rbinfo.shape = m_phyWorld->CreateCapsuleShape(
				BI.body.size.x, ( BI.body.size.z - BI.body.size.x ) * 2 );
		}
		else if( BI.body.type == AnimCharacter::BodyType_Box )
		{
			rbinfo.shape = m_phyWorld->CreateBoxShape( BI.body.size );
		}
		else continue;
		rbinfo.mass = 2;//4.0f / 3.0f * M_PI * SB.capsule_radius * SB.capsule_radius * SB.capsule_radius + M_PI * SB.capsule_radius * SB.capsule_radius * SB.capsule_height;
		
		rbinfo.inertia = rbinfo.shape->CalcInertia( rbinfo.mass );
		
		TB->relPos = BI.body.position;
		TB->relRot = BI.body.rotation;
		TB->bodyHandle = m_phyWorld->CreateRigidBody( rbinfo );
	}
	
	for( size_t bid = 0; bid < chinfo->bones.size(); ++bid )
	{
		const AnimCharacter::BoneInfo& BI = chinfo->bones[ bid ];
		
		int jpbid = BI.joint.parent_id;
		if( jpbid >= 0 && BI.joint.type != AnimCharacter::JointType_None )
		{
			Mat4 jsm, jpm;
			if( m_bones[ chinst->m_bone_ids[ bid ] ].bodyHandle &&
				m_bones[ chinst->m_bone_ids[ jpbid ] ].bodyHandle &&
				chinfo->GetJointFrameMatrices( bid, jsm, jpm ) )
			{
				if( BI.joint.type == AnimCharacter::JointType_Hinge )
				{
					SGRX_PhyHingeJointInfo jinfo;
					jinfo.enabled = false;
					jinfo.bodyA = m_bones[ chinst->m_bone_ids[ bid ] ].bodyHandle;
					jinfo.bodyB = m_bones[ chinst->m_bone_ids[ jpbid ] ].bodyHandle;
					jinfo.frameA = jsm;
					jinfo.frameB = jpm;
					m_bones[ chinst->m_bone_ids[ bid ] ].jointHandle =
						m_phyWorld->CreateHingeJoint( jinfo );
				}
				else if( BI.joint.type == AnimCharacter::JointType_ConeTwist )
				{
					SGRX_PhyConeTwistJointInfo jinfo;
					jinfo.enabled = false;
					jinfo.bodyA = m_bones[ chinst->m_bone_ids[ bid ] ].bodyHandle;
					jinfo.bodyB = m_bones[ chinst->m_bone_ids[ jpbid ] ].bodyHandle;
					jinfo.frameA = jsm;
					jinfo.frameB = jpm;
					jinfo.coneLimitX = DEG2RAD( BI.joint.turn_limit_1 );
					jinfo.coneLimitY = DEG2RAD( BI.joint.turn_limit_2 );
					jinfo.twistLimit = DEG2RAD( BI.joint.twist_limit );
					m_bones[ chinst->m_bone_ids[ bid ] ].jointHandle =
						m_phyWorld->CreateConeTwistJoint( jinfo );
				}
			}
		}
	}
}

void AnimRagdoll::Prepare()
{
	m_bones.resize( m_pose.size() );
	for( size_t i = 0; i < m_pose.size(); ++i )
	{
		Body B = { V3(0), Quat::Identity,
			NULL, NULL, V3(0), V3(0), Quat::Identity, Quat::Identity };
		m_bones[ i ] = B;
		m_pose[ i ].Reset();
	}
}

void AnimRagdoll::Advance( float deltaTime, AnimInfo* info )
{
	m_lastTickSize = deltaTime;
	
	ASSERT( m_bones.size() == m_pose.size() );
	SGRX_MeshBone* MB = m_mesh.GetBonePtr();
	for( size_t i = 0; i < m_pose.size(); ++i )
		m_pose[ i ].fq = m_enabled; // && ( m_bones[ i ].bodyHandle || MB[ i ].parent_id >= 0 );
	
	if( m_enabled == false )
		return;
	
	for( size_t i = 0; i < m_bones.size(); ++i )
	{
		Body& B = m_bones[ i ];
		if( B.bodyHandle )
		{
			Vec3 pos = B.bodyHandle->GetPosition();
			Quat rot = B.bodyHandle->GetRotation();
			Quat nrot = B.relRot.Inverted() * rot;
			m_pose[ i ].pos = pos - Mat4::CreateRotationFromQuat(nrot).TransformNormal( B.relPos );
			m_pose[ i ].rot = nrot;
		}
		else if( MB[ i ].parent_id >= 0 )
		{
			m_pose[ i ].SetMatrix( MB[ i ].boneOffset * m_pose[ MB[ i ].parent_id ].GetSRT() );
		}
	}
}

void AnimRagdoll::SetBoneTransforms( int bone_id, const Vec3& prev_pos, const Vec3& curr_pos, const Quat& prev_rot, const Quat& curr_rot )
{
	ASSERT( bone_id >= 0 && bone_id < (int) m_pose.size() );
	Body& B = m_bones[ bone_id ];
	B.prevPos = prev_pos;
	B.currPos = curr_pos;
	B.prevRot = prev_rot;
	B.currRot = curr_rot;
}

void AnimRagdoll::AdvanceTransforms( Animator* anim )
{
	if( m_pose.size() != anim->m_pose.size() )
		return;
	
	for( size_t i = 0; i < m_pose.size(); ++i )
	{
		Body& B = m_bones[ i ];
		B.prevPos = B.currPos;
		B.prevRot = B.currRot;
		B.currPos = anim->m_pose[ i ].pos;
		B.currRot = anim->m_pose[ i ].rot;
	}
}

void AnimRagdoll::EnablePhysics( const Mat4& worldMatrix )
{
	if( m_enabled )
		return;
	m_enabled = true;
	
	Mat4 prev_boneToWorldMatrices[ SGRX_MAX_MESH_BONES ];
	Mat4 curr_boneToWorldMatrices[ SGRX_MAX_MESH_BONES ];
	SGRX_MeshBone* MB = m_mesh->m_bones;
	for( size_t i = 0; i < m_bones.size(); ++i )
	{
		Body& B = m_bones[ i ];
		Mat4& prev_M = prev_boneToWorldMatrices[ i ];
		Mat4& curr_M = curr_boneToWorldMatrices[ i ];
		prev_M = Mat4::CreateSRT( V3(1), B.prevRot, B.prevPos ) * MB[ i ].boneOffset;
		curr_M = Mat4::CreateSRT( V3(1), B.currRot, B.currPos ) * MB[ i ].boneOffset;
		if( MB[ i ].parent_id >= 0 )
		{
			prev_M = prev_M * prev_boneToWorldMatrices[ MB[ i ].parent_id ];
			curr_M = curr_M * curr_boneToWorldMatrices[ MB[ i ].parent_id ];
		}
		else
		{
			prev_M = prev_M * worldMatrix;
			curr_M = curr_M * worldMatrix;
		}
	}
	
	for( size_t i = 0; i < m_bones.size(); ++i )
	{
		Body& B = m_bones[ i ];
		if( B.bodyHandle )
		{
			// calculate world transforms
			Mat4 rtf = Mat4::CreateSRT( V3(1), B.relRot, B.relPos );
			Mat4 prev_wtf = rtf * prev_boneToWorldMatrices[ i ];
			Mat4 curr_wtf = rtf * curr_boneToWorldMatrices[ i ];
			
			Vec3 prev_wpos = prev_wtf.GetTranslation();
			Vec3 curr_wpos = curr_wtf.GetTranslation();
			Quat prev_wrot = prev_wtf.GetRotationQuaternion();
			Quat curr_wrot = curr_wtf.GetRotationQuaternion();
			
			B.bodyHandle->SetPosition( curr_wpos );
			B.bodyHandle->SetRotation( curr_wrot );
			if( m_lastTickSize > SMALL_FLOAT )
			{
				float speed = 1.0f / m_lastTickSize;
				B.bodyHandle->SetLinearVelocity( ( curr_wpos - prev_wpos ) * speed );
				B.bodyHandle->SetAngularVelocity( CalcAngularVelocity( prev_wrot, curr_wrot ) * speed );
			}
			B.bodyHandle->SetEnabled( true );
		}
		if( B.jointHandle )
		{
			B.jointHandle->SetEnabled( true );
		}
	}
}

void AnimRagdoll::DisablePhysics()
{
	if( m_enabled == false )
		return;
	m_enabled = false;
	for( size_t i = 0; i < m_bones.size(); ++i )
	{
		Body& B = m_bones[ i ];
		if( B.jointHandle )
			B.jointHandle->SetEnabled( false );
		if( B.bodyHandle )
			B.bodyHandle->SetEnabled( false );
	}
}

void AnimRagdoll::WakeUp()
{
	for( size_t i = 0; i < m_bones.size(); ++i )
	{
		Body& B = m_bones[ i ];
		if( B.bodyHandle )
			B.bodyHandle->WakeUp();
	}
}

void AnimRagdoll::ApplyImpulse( Vec3 origin, Vec3 imp, size_t bone )
{
	if( bone < m_bones.size() )
	{
		if( m_bones[ bone ].bodyHandle )
			m_bones[ bone ].bodyHandle->ApplyForce( PFT_Impulse, imp, origin );
	}
	else
	{
		for( size_t i = 0; i < m_bones.size(); ++i )
		{
			if( m_bones[ i ].bodyHandle )
				m_bones[ i ].bodyHandle->ApplyForce( PFT_Impulse, imp, origin );
		}
	}
}

static float _CalcAtten( AnimRagdoll::Body& B, float att, float rad, Vec3 p )
{
	Vec3 bp = B.bodyHandle->GetPosition();
	float dist = ( p - bp ).Length();
	return powf( clamp( safe_fdiv( rad - dist, rad ), 0, 1 ), att );
}

void AnimRagdoll::ApplyImpulseExt( Vec3 origin, Vec3 imp, float atten, float radius, size_t bone )
{
	if( bone < m_bones.size() )
	{
		if( m_bones[ bone ].bodyHandle )
		{
			float a = _CalcAtten( m_bones[ bone ], atten, radius, origin );
			m_bones[ bone ].bodyHandle->ApplyForce( PFT_Impulse, imp * a, origin );
		}
	}
	else
	{
		for( size_t i = 0; i < m_bones.size(); ++i )
		{
			if( m_bones[ i ].bodyHandle )
			{
				float a = _CalcAtten( m_bones[ i ], atten, radius, origin );
				m_bones[ i ].bodyHandle->ApplyForce( PFT_Impulse, imp * a, origin );
			}
		}
	}
}



void AnimRotator::Advance( float deltaTime, AnimInfo* info )
{
	ANIMATOR_ADVANCE_FRAME_CHECK( info );
	if( animSource )
	{
		animSource->Advance( deltaTime, info );
		Mat4 rm = Mat4::CreateRotationZ( DEG2RAD( angle ) );
		for( size_t i = 0; i < m_pose.size(); ++i )
		{
			Mat4 xf = animSource->m_pose[ i ].GetSRT();
			Vec3 tr = animSource->m_pose[ 0 ].pos; // origin position
			xf.SetTranslation( xf.GetTranslation() - tr );
			xf = M4MulAff( xf, rm );
			xf.SetTranslation( xf.GetTranslation() + tr );
			m_pose[ i ].SetMatrix( xf );
			m_pose[ i ].fq = animSource->m_pose[ i ].fq;
		}
	}
}



MECompileResult AnimCharacter::ValExpr::Recompile( const MEVariableInterface* vars )
{
	return compiled_expr.Compile( expr, vars );
}

struct RHTKey
{
	SGRX_GUID guid;
	size_t id;
	
	static int sortfn( const void* a, const void* b )
	{
		SGRX_CAST( RHTKey*, A, a );
		SGRX_CAST( RHTKey*, B, b );
		return A->guid.Compare( B->guid );
	}
};

void AnimCharacter::PlayerNode::RehashStates()
{
	state_lookup.clear();
	for( size_t i = 0; i < states.size(); ++i )
		state_lookup.set( states[ i ]->guid, states[ i ] );
}

void AnimCharacter::PlayerNode::RehashTransitions()
{
	transition_lookup.clear();
	transition_lookup_ids.clear();
	
	Array< RHTKey > keys;
	for( size_t i = 0; i < transitions.size(); ++i )
	{
		const Handle<Transition>& T = transitions[ i ];
		RHTKey key = { T->source, i };
		keys.push_back( key );
		if( T->source.NotNull() && T->target.NotNull() && T->bidi )
		{
			key.guid = T->target;
			keys.push_back( key );
		}
	}
	if( keys.size() == 0 )
	{
		// ensure there is NULL GUID lookup info at the beginning
		transition_lookup_ids.push_back( 0 );
		return;
	}
	
	// sort by GUID
	qsort( keys.data(), keys.size(), sizeof(RHTKey), RHTKey::sortfn );
	
	// ensure there is NULL GUID lookup info at the beginning
	if( keys[0].guid.NotNull() )
		transition_lookup_ids.push_back( 0 );
	
	// index the data
	static const uint8_t not_null_guid[16] = {1};
	SGRX_GUID prev_guid = keys[0].guid.NotNull() ?
		SGRX_GUID::Null : SGRX_GUID::FromBytes( not_null_guid );
	size_t curr_count_at = 0;
	for( size_t i = 0; i < keys.size(); ++i )
	{
		if( keys[ i ].guid != prev_guid )
		{
			prev_guid = keys[ i ].guid;
			curr_count_at = transition_lookup_ids.size();
			transition_lookup.set( prev_guid, curr_count_at );
			transition_lookup_ids.push_back( 0 );
		}
		transition_lookup_ids[ curr_count_at ]++;
		transition_lookup_ids.push_back( keys[ i ].id );
	}
}



bool AnimCharacter::Load( const StringView& sv )
{
	ByteArray ba;
	if( !FS_LoadBinaryFile( sv, ba ) )
		return false;
	ByteReader br( ba );
	Serialize( br );
	if( br.error )
		return false;
	
	_Prepare();
	return true;
}

bool AnimCharacter::Save( const StringView& sv )
{
	ByteArray ba;
	ByteWriter bw( &ba );
	Serialize( bw );
	return FS_SaveBinaryFile( sv, ba.data(), ba.size() );
}

void AnimCharacter::_Prepare()
{
	_RecalcBoneIDs();
	_ReindexVariables();
	_RecompileExpressions();
}

void AnimCharacter::_RecalcBoneIDs()
{
	for( size_t i = 0; i < bones.size(); ++i )
	{
		BoneInfo& BI = bones[ i ];
		BI.joint.parent_id = _FindBoneByName( BI.joint.parent_name );
	}
	for( size_t i = 0; i < attachments.size(); ++i )
	{
		Attachment& AT = attachments[ i ];
		AT.bone_id = _FindBoneByName( AT.bone );
	}
}

void AnimCharacter::_ReindexVariables()
{
	m_variable_index.clear();
	for( size_t i = 0; i < variables.size(); ++i )
	{
		m_variable_index.set( variables[ i ]->name, i );
	}
}

void AnimCharacter::_RecompileExpressions()
{
	for( size_t i = 0; i < aliases.size(); ++i )
		aliases[ i ]->expr.Recompile( this );
	for( size_t i = 0; i < nodes.size(); ++i )
	{
		Node* N = nodes[ i ];
		if( N->type == NT_Player )
		{
			SGRX_CAST( PlayerNode*, PN, N );
			for( size_t j = 0; j < PN->states.size(); ++j )
			{
				State* s = PN->states[ j ];
				s->speed.Recompile( this );
			}
			for( size_t j = 0; j < PN->transitions.size(); ++j )
			{
				Transition* tr = PN->transitions[ j ];
				tr->expr.Recompile( this );
			}
		}
		else if( N->type == NT_Blend )
		{
			SGRX_CAST( BlendNode*, BN, N );
			BN->factor.Recompile( this );
		}
		else if( N->type == NT_Rotator )
		{
			SGRX_CAST( RotatorNode*, RN, N );
			RN->angle.Recompile( this );
		}
	}
}

void AnimCharacter::_UnlinkNode( Node* node )
{
	for( size_t i = 0; i < nodes.size(); ++i )
	{
		for( int l = 0; l < nodes[ i ]->GetInputLinkCount(); ++l )
		{
			SGRX_GUID* pguid = nodes[ i ]->GetInputLink( l );
			if( *pguid == node->guid )
				pguid->SetNull();
		}
	}
}

int AnimCharacter::_FindBoneByName( const StringView& name )
{
	for( size_t i = 0; i < bones.size(); ++i )
	{
		if( bones[ i ].name == name )
			return i;
	}
	return -1;
}

AnimCharacter::Node* AnimCharacter::_FindNodeByGUID( SGRX_GUID guid )
{
	for( size_t i = 0; i < nodes.size(); ++i )
	{
		if( nodes[ i ]->guid == guid )
			return nodes[ i ];
	}
	return NULL;
}


#define ANIMCHAR_VAR_time   0
#define ANIMCHAR_VAR_pos    1
#define ANIMCHAR_VAR_length 2
#define ANIMCHAR_VAR_end    3
#define ANIMCHAR_VAR_nfba   4
#define ANIMCHAR_VAR_said   5

#define ANIMCHAR_VAR_VOFF (6)
#define ANIMCHAR_VAR_AOFF (6 + animChar->variables.size())

uint16_t AnimCharacter::MEGetID( StringView name ) const
{
	if( name.ch() == '_' )
	{
		if( name == "_time" ) return 0;
		if( name == "_pos" ) return 1;
		if( name == "_length" ) return 2;
		if( name == "_end" ) return 3;
		if( name == "_nfba" ) return 4;
		if( name == "_said" ) return 5;
	}
	uint16_t off = 6;
	for( size_t i = 0; i < variables.size(); ++i )
	{
		if( variables[ i ]->name == name )
			return uint16_t( i ) + off;
	}
	off += variables.size();
	for( size_t i = 0 ; i < aliases.size(); ++i )
	{
		if( aliases[ i ]->name == name )
			return uint16_t( i ) + off;
	}
	return ME_OPERAND_NONE;
}


bool AnimCharacter::ApplyMask( SGRX_IMesh* mesh, const StringView& name, AnimMask* tgt )
{
	ASSERT( mesh );
	for( size_t i = 0; i < masks.size(); ++i )
	{
		Mask& M = masks[ i ];
		if( M.name != name )
			continue;
		
		ArrayView<float> factors = tgt->blendFactors;
		GR_ClearFactors( factors, 0 );
		for( size_t j = 0; j < M.cmds.size(); ++j )
		{
			MaskCmd& MC = M.cmds[ j ];
			GR_SetFactors( factors, mesh, MC.bone, MC.weight, MC.children, MC.mode );
		}
		return true;
	}
	return false;
}

bool AnimCharacter::GetJointFrameMatrices( int which, Mat4& outself, Mat4& outprnt )
{
	if( which < 0 || which >= (int) bones.size() )
		return false;
	BoneInfo& BI = bones[ which ];
	int pb = BI.joint.parent_id;
	if( pb < 0 || pb >= (int) bones.size() )
		return false;
	BoneInfo& PBI = bones[ pb ];
	
	// calc inverse body matrices
	Mat4 bm = Mat4::CreateRotationFromQuat( BI.body.rotation ) *
		Mat4::CreateTranslation( BI.body.position ),
		pbm = Mat4::CreateRotationFromQuat( PBI.body.rotation ) *
		Mat4::CreateTranslation( PBI.body.position );
	Mat4 ibm = Mat4::Identity, ipbm = Mat4::Identity;
	bm.InvertTo( ibm );
	pbm.InvertTo( ipbm );
	
	// calc joint matrices
	outself = Mat4::CreateRotationFromQuat( BI.joint.self_rotation ) *
		Mat4::CreateTranslation( BI.joint.self_position ) * ibm;
	outprnt = Mat4::CreateRotationFromQuat( BI.joint.prnt_rotation ) *
		Mat4::CreateTranslation( BI.joint.prnt_position ) * ipbm;
	return true;
}

int AnimCharacter::FindAttachment( const StringView& name )
{
	for( size_t i = 0; i < attachments.size(); ++i )
	{
		if( attachments[ i ].name == name )
			return i;
	}
	return -1;
}




Animator* AnimCharInst::RagdollNodeRT::GetAnimator( AnimCharInst* ch )
{
	return &ch->m_anRagdoll;
}

void AnimCharInst::PlayerNodeRT::StartCurrentState()
{
	if( !current_state )
		return;
	player_anim.Play(
		GR_GetAnim( current_state->anim ),
		current_state->playMode,
		current_state->fade_time,
		current_state->fade_speed );
	m_stateTime = 0;
}

void AnimCharInst::PlayerNodeRT::UpdateState( const MEVariableInterface* vars )
{
	SGRX_CAST( AnimCharacter::PlayerNode*, PN, src );
	
	if( !current_state )
	{
		current_state = PN->starting_state;
		StartCurrentState();
	}
	if( !current_state )
		return;
	
	// from-any transitions
	for( size_t i = 0; i < PN->transition_lookup_ids[0]; ++i )
	{
		AnimCharacter::Transition* tr = PN->transitions[ PN->transition_lookup_ids[ i + 1 ] ];
		if( tr->expr.Eval( vars ) )
		{
			current_state = PN->state_lookup.getcopy( tr->target );
			StartCurrentState();
			return;
		}
	}
	
	// directly relevant transitions
	size_t off = PN->transition_lookup[ current_state->guid ];
	for( size_t i = 0; i < PN->transition_lookup_ids[ off ]; ++i )
	{
		AnimCharacter::Transition* tr = PN->transitions[ PN->transition_lookup_ids[ i + off + 1 ] ];
		if( tr->source == current_state->guid )
		{
			if( tr->expr.Eval( vars ) )
			{
				current_state = PN->state_lookup.getcopy( tr->target );
				StartCurrentState();
				return;
			}
		}
		else if( tr->bidi && tr->target == current_state->guid )
		{
			if( !tr->expr.Eval( vars ) )
			{
				current_state = PN->state_lookup.getcopy( tr->source );
				StartCurrentState();
				return;
			}
		}
	}
}



AnimCharInst::AnimCharInst( SceneHandle sh, PhyWorldHandle phyWorld ) :
	m_scene( sh ),
	m_frameID( 0 ),
	m_animTimeLeft( 0 ),
	m_anRagdoll( phyWorld )
{
	ASSERT( sh && "scene handle must be valid" );
	
	m_cachedMeshInst = m_scene->CreateMeshInstance();
	m_cachedMeshInst->raycastOverride = this;
	skinName = "!default";
	
	_Prepare();
}

bool AnimCharInst::SetAnimChar( const StringView& sv )
{
	AnimCharHandle ch = GR_GetAnimChar( sv );
	SetAnimChar( ch );
	return ch != NULL;
}

void AnimCharInst::SetAnimChar( AnimCharacter* ch )
{
	animChar = ch;
	_OnRenderUpdate();
}

void AnimCharInst::SetSkin( StringView name )
{
	skinName = name;
	_OnRenderUpdate( true );
}

void AnimCharInst::_OnRenderUpdate( bool skinOnly )
{
	AnimCharacter::Skin* skin;
	if( animChar && skinName == SV("!default") )
		m_cachedMesh = GR_GetMesh( animChar->mesh );
	else if( animChar && ( skin = animChar->skins.getptr( skinName ) ) != NULL )
		m_cachedMesh = GR_GetMesh( skin->mesh );
	else
		m_cachedMesh = NULL;
	m_cachedMeshInst->SetMesh( m_cachedMesh );
	m_cachedMeshInst->skin_matrices.resize( m_cachedMesh.GetBoneCount() );
	m_cachedMeshInst->raycastOverride = m_cachedMesh.GetBoneCount() ? this : NULL;
	_RecalcBoneIDs();
	_Prepare( skinOnly );
	m_anRagdoll.Initialize( this );
}

void AnimCharInst::_Prepare( bool skinOnly )
{
	if( !skinOnly ) // no need to reset variables / nodes on skin change
	{
		// prepare variable storage & initial data
		m_rtnodes.clear();
		if( animChar )
		{
			m_values.resize( 6 + animChar->variables.size() + animChar->aliases.size() );
			for( size_t i = 0; i < animChar->variables.size(); ++i )
				m_values[ ANIMCHAR_VAR_VOFF + i ] = animChar->variables[ i ]->value;
			
			for( size_t i = 0; i < animChar->nodes.size(); ++i )
			{
				AnimCharacter::Node* N = animChar->nodes[ i ];
				NodeRT* NRT = NULL;
				switch( N->type )
				{
				case AnimCharacter::NT_Player: {
					SGRX_CAST( AnimCharacter::PlayerNode*, PN, N );
					PlayerNodeRT* PNRT = new PlayerNodeRT;
					PNRT->current_state = PN->starting_state;
					NRT = PNRT;
				} break;
				case AnimCharacter::NT_Blend: NRT = new BlendNodeRT; break;
				case AnimCharacter::NT_Ragdoll: NRT = new RagdollNodeRT; break;
				case AnimCharacter::NT_Mask: NRT = new MaskNodeRT; break;
				case AnimCharacter::NT_RelAbs: NRT = new RelAbsNodeRT; break;
				case AnimCharacter::NT_Rotator: NRT = new RotatorNodeRT; break;
				}
				NRT->src = N;
				m_rtnodes.push_back( NRT );
			}
		}
		else m_values.resize( 6 );
		_PrepareSpecialVariables( NULL );
	}
	
	// count animators
	int num_animators = 2; // ragdoll, end
	int num_bones = m_cachedMesh.GetBoneCount();
	for( size_t i = 0; i < m_rtnodes.size(); ++i )
	{
		if( m_rtnodes[ i ]->OwnsAnimator() )
			num_animators++;
	}
	
	// allocate frames
	m_node_frames.resize( num_animators * num_bones );
	for( size_t i = 0; i < m_node_frames.size(); ++i )
		m_node_frames[ i ].Reset();
	
	// equip animators
	int at = 0;
	_EquipAnimator( &m_anRagdoll, at++ );
	_EquipAnimator( &m_anEnd, at++ );
	for( size_t i = 0; i < m_rtnodes.size(); ++i )
	{
		Animator* anim = m_rtnodes[ i ]->GetAnimator( this );
		if( anim && m_rtnodes[ i ]->OwnsAnimator() )
			_EquipAnimator( anim, at++ );
	}
	
	// link & set up masks
	if( !skinOnly )
		m_anEnd.animSource = NULL;
	for( size_t i = 0; i < m_rtnodes.size(); ++i )
	{
		NodeRT* NRT = m_rtnodes[ i ];
		if( NRT->src->type == AnimCharacter::NT_Mask && m_cachedMesh )
		{
			SGRX_CAST( AnimCharacter::MaskNode*, MN, NRT->src );
			SGRX_CAST( MaskNodeRT*, MNRT, NRT );
			animChar->ApplyMask( m_cachedMesh, MN->mask_name, &MNRT->mask_anim );
		}
		if( !skinOnly ) // no need to relink animators on skin change
		{
			for( int l = 0; l < m_rtnodes[ i ]->src->GetInputLinkCount(); ++l )
			{
				const SGRX_GUID* pguid = m_rtnodes[ i ]->src->GetInputLink( l );
				Animator** panim = m_rtnodes[ i ]->GetInputSource( l );
				
				// -- iterate nodes to find one with matching GUID
				for( size_t j = 0; j < m_rtnodes.size(); ++j )
				{
					if( m_rtnodes[ j ]->src->guid == *pguid )
					{
						*panim = m_rtnodes[ j ]->GetAnimator( this );
						break;
					}
				}
			}
			if( m_rtnodes[ i ]->src == animChar->output_node )
				m_anEnd.animSource = m_rtnodes[ i ]->GetAnimator( this );
		}
	}
}

void AnimCharInst::_EquipAnimator( Animator* anim, int which )
{
	int num_bones = m_cachedMesh.GetBoneCount();
	int at = which * num_bones;
	anim->m_mesh = m_cachedMesh;
	anim->m_pose = ArrayView<AnimTrackFrame>( m_node_frames ).part( at, num_bones );
	anim->Prepare();
}

void AnimCharInst::_PrepareSpecialVariables( NodeRT* n )
{
	if( n )
	{
		if( n->src->type == AnimCharacter::NT_Player )
		{
			SGRX_CAST( PlayerNodeRT*, PN, n );
			if( PN->current_state )
			{
				AnimPlayer::Anim* panim = NULL;
				if( PN->player_anim.m_currentAnims.size() )
					panim = &PN->player_anim.m_currentAnims.last();
				SGRX_Animation* aanim = panim ? panim->anim : NULL;
				
				StringView cur_state_anim = PN->current_state->anim;
				StringView cur_played_anim = aanim ? SV(aanim->m_key) : SV();
				bool still_playing = cur_state_anim == cur_played_anim;
				
				m_values[ ANIMCHAR_VAR_time ] = PN->m_stateTime;
				m_values[ ANIMCHAR_VAR_pos ] = still_playing && panim ? panim->fade_at : 0;
				m_values[ ANIMCHAR_VAR_length ] = still_playing && aanim ? aanim->GetAnimTime() : 0;
				m_values[ ANIMCHAR_VAR_end ] = !still_playing || ( m_values[ ANIMCHAR_VAR_pos ] > m_values[ ANIMCHAR_VAR_length ] );
				return;
			}
		}
	}
	
	// defaults
	{
		m_values[ ANIMCHAR_VAR_time ] = 0;
		m_values[ ANIMCHAR_VAR_pos ] = 0;
		m_values[ ANIMCHAR_VAR_length ] = 0;
		m_values[ ANIMCHAR_VAR_end ] = false;
	}
}

void AnimCharInst::_RecalcBoneIDs()
{
	size_t bs = animChar ? animChar->bones.size() : 0;
	m_bone_ids.resize( bs );
	for( size_t i = 0; i < bs; ++i )
	{
		const AnimCharacter::BoneInfo& BI = animChar->bones[ i ];
		m_bone_ids[ i ] = m_cachedMesh.FindBone( BI.name );
	}
}


void AnimCharInst::SetTransform( const Mat4& mtx )
{
	if( m_cachedMeshInst )
		m_cachedMeshInst->matrix = mtx;
}

void AnimCharInst::FixedTick( float deltaTime, bool changeStates )
{
	LOG_FUNCTION_ARG("AnimCharInst");
	
	m_animTimeLeft -= deltaTime;
	m_values[ ANIMCHAR_VAR_nfba ] = 0;
	if( m_animTimeLeft > 0 )
	{
		changeStates = false;
		if( animChar->main_player_node && animChar->main_player_node->type == AnimCharacter::NT_Player )
		{
			for( size_t i = 0; i < m_rtnodes.size(); ++i )
			{
				if( m_rtnodes[ i ]->src == animChar->main_player_node )
				{
					SGRX_CAST( PlayerNodeRT*, PNRT, m_rtnodes[ i ].item );
					m_values[ ANIMCHAR_VAR_nfba ] = PNRT->player_anim.GetLastAnimBlendFactor();
					break;
				}
			}
		}
	}
	else m_values[ ANIMCHAR_VAR_said ] = 0;
	
	_PrepareSpecialVariables( NULL );
	for( size_t i = 0; i < animChar->aliases.size(); ++i )
	{
		m_values[ ANIMCHAR_VAR_AOFF + i ] = animChar->aliases[ i ]->expr.Eval( this );
	}
	
	for( size_t i = 0; i < m_rtnodes.size(); ++i )
	{
		if( m_rtnodes[ i ]->src->type == AnimCharacter::NT_Player && changeStates )
		{
			SGRX_CAST( PlayerNodeRT*, PNRT, m_rtnodes[ i ].item );
			_PrepareSpecialVariables( PNRT );
			PNRT->UpdateState( this );
		}
		m_rtnodes[ i ]->Advance( deltaTime, this );
	}
	
	AnimInfo info = { ++m_frameID, m_cachedMeshInst->matrix };
	m_anEnd.Advance( deltaTime, &info );
	m_anRagdoll.AdvanceTransforms( &m_anEnd );
}

void AnimCharInst::PreRender( float blendFactor )
{
	LOG_FUNCTION_ARG("AnimCharInst");
	
	m_anEnd.Interpolate( blendFactor );
	GR_ApplyAnimator( &m_anEnd, m_cachedMeshInst );
}

void AnimCharInst::ResetStates()
{
	for( size_t i = 0; i < m_rtnodes.size(); ++i )
	{
		if( m_rtnodes[ i ]->src->type == AnimCharacter::NT_Player )
		{
			SGRX_CAST( PlayerNodeRT*, PNRT, m_rtnodes[ i ].item );
			SGRX_CAST( AnimCharacter::PlayerNode*, PN, PNRT->src );
			PNRT->current_state = PN->starting_state;
			PNRT->StartCurrentState();
		}
	}
}

void AnimCharInst::EnablePhysics()
{
	m_anRagdoll.EnablePhysics( m_cachedMeshInst->matrix );
}

void AnimCharInst::DisablePhysics()
{
	m_anRagdoll.DisablePhysics();
}

void AnimCharInst::WakeUp()
{
	m_anRagdoll.WakeUp();
}


bool AnimCharInst::CheckMarker( const StringView& name )
{
	for( size_t i = 0; i < m_rtnodes.size(); ++i )
	{
		NodeRT* N = m_rtnodes[ i ];
		if( N->src->type == AnimCharacter::NT_Player )
		{
			SGRX_CAST( PlayerNodeRT*, PNRT, N );
			if( PNRT->player_anim.CheckMarker( name ) )
				return true;
		}
	}
	return false;
}

bool AnimCharInst::IsPlayingAnim() const
{
	return m_animTimeLeft > 0;
}

void AnimCharInst::PlayAnim( StringView name, bool loop )
{
	AnimCharacter::MappedAnim* maptr = animChar->mapping.getptr( name );
	if( maptr )
	{
		name = maptr->anim;
		m_values[ ANIMCHAR_VAR_said ] = maptr->id;
	}
	else m_values[ ANIMCHAR_VAR_said ] = 0;
	
	AnimHandle anim = GR_GetAnim( name );
	if( anim == NULL )
	{
		LOG_WARNING << "AnimCharInst::PlayAnim - anim not found: " << name;
		return;
	}
	if( animChar->main_player_node == NULL || animChar->main_player_node->type != AnimCharacter::NT_Player )
	{
		LOG_WARNING << "AnimCharInst::PlayAnim - main player node undefined";
		return;
	}
	m_animTimeLeft = loop ? FLT_MAX : anim->GetAnimTime();
	for( size_t i = 0; i < m_rtnodes.size(); ++i )
	{
		if( m_rtnodes[ i ]->src->type != AnimCharacter::NT_Player )
			continue;
		SGRX_CAST( PlayerNodeRT*, PNRT, m_rtnodes[ i ].item );
		PNRT->player_anim.Play( PNRT->src == animChar->main_player_node ? anim : NULL, !loop );
	}
}

void AnimCharInst::StopAnim()
{
	if( m_animTimeLeft > 0 )
	{
		m_animTimeLeft = 0;
		// animation will be changed on next tick
	}
}


float* AnimCharInst::_GetVarValAt( int i )
{
	if( size_t(i) >= animChar->variables.size() )
		return NULL;
	return &m_values[ ANIMCHAR_VAR_VOFF + i ];
}

void AnimCharInst::_SetVar( StringView name, float val )
{
	uint16_t* id = animChar->m_variable_index.getptr( name );
	if( !id || size_t(*id) >= animChar->variables.size() )
		return;
	m_values[ ANIMCHAR_VAR_VOFF + *id ] = val;
}


int AnimCharInst::_FindParentBone( int which )
{
	if( m_cachedMesh == NULL )
		return -1;
	if( which < 0 || which >= (int) animChar->bones.size() )
		return -1;
	
	int parent_id = m_cachedMesh->m_bones[ _GetMeshBoneID( which ) ].parent_id;
	for( size_t i = 0; i < animChar->bones.size(); ++i )
	{
		if( m_bone_ids[ i ] == parent_id )
			return i;
	}
	return -1;
}

bool AnimCharInst::_GetBodyMatrix( int which, Mat4& outwm )
{
	if( !m_cachedMesh )
		return false;
	if( which < 0 || which >= (int) animChar->bones.size() )
		return false;
	const AnimCharacter::BoneInfo& BI = animChar->bones[ which ];
	
	outwm = m_cachedMeshInst->matrix;
	if( _GetMeshBoneID( which ) >= 0 )
	{
		if( m_cachedMeshInst->IsSkinned() )
		{
			outwm = m_cachedMeshInst->skin_matrices[ _GetMeshBoneID( which ) ] * outwm;
		}
		outwm = m_cachedMesh->m_bones[ _GetMeshBoneID( which ) ].skinOffset * outwm;
	}
	outwm = Mat4::CreateRotationFromQuat( BI.body.rotation ) *
		Mat4::CreateTranslation( BI.body.position ) * outwm;
	return true;
}

bool AnimCharInst::_GetJointMatrix( int which, bool parent, Mat4& outwm )
{
	if( !m_cachedMesh )
		return false;
	if( which < 0 || which >= (int) animChar->bones.size() )
		return false;
	const AnimCharacter::BoneInfo& BI = animChar->bones[ which ];
	int bid = _GetMeshBoneID( which );
	if( parent )
	{
		int pb = BI.joint.parent_id;
		if( pb < 0 || pb >= (int) animChar->bones.size() )
			return false;
		bid = _GetMeshBoneID( pb );
	}
	
	outwm = m_cachedMeshInst->matrix;
	if( bid >= 0 )
	{
		if( m_cachedMeshInst->IsSkinned() )
		{
			outwm = m_cachedMeshInst->skin_matrices[ bid ] * outwm;
		}
		outwm = m_cachedMesh->m_bones[ bid ].skinOffset * outwm;
	}
	if( parent )
	{
		outwm = Mat4::CreateRotationFromQuat( BI.joint.prnt_rotation ) *
			Mat4::CreateTranslation( BI.joint.prnt_position ) * outwm;
	}
	else
	{
		outwm = Mat4::CreateRotationFromQuat( BI.joint.self_rotation ) *
			Mat4::CreateTranslation( BI.joint.self_position ) * outwm;
	}
	return true;
}

void AnimCharInst::_GetHitboxMatrix( int which, Mat4& outwm )
{
	const AnimCharacter::BoneInfo& BI = animChar->bones[ which ];
	outwm = Mat4::CreateRotationFromQuat( BI.hitbox.rotation ) *
		Mat4::CreateTranslation( BI.hitbox.position );
	if( _GetMeshBoneID( which ) >= 0 )
	{
		outwm = outwm * m_cachedMesh->m_bones[ _GetMeshBoneID( which ) ].skinOffset;
		if( m_cachedMeshInst->IsSkinned() )
		{
			outwm = outwm * m_cachedMeshInst->skin_matrices[ _GetMeshBoneID( which ) ];
		}
	}
}

bool AnimCharInst::_GetHitboxOBB( int which, Mat4& outwm, Vec3& outext )
{
	if( !m_cachedMesh )
		return false;
	if( which < 0 || which >= (int) animChar->bones.size() )
		return false;
	const AnimCharacter::BoneInfo& BI = animChar->bones[ which ];
	if( BI.hitbox.multiplier == 0 )
		return false; // a way to disable it
	
	_GetHitboxMatrix( which, outwm );
	outwm = outwm * m_cachedMeshInst->matrix;
	outext = BI.hitbox.extents;
	return true;
}


bool AnimCharInst::GetAttachmentMatrix( int which, Mat4& outwm, bool worldspace ) const
{
	outwm = worldspace ? m_cachedMeshInst->matrix : Mat4::Identity;
	if( !m_cachedMesh )
		return false;
	if( which < 0 || which >= (int) animChar->attachments.size() )
		return false;
	const AnimCharacter::Attachment& AT = animChar->attachments[ which ];
	
	if( AT.bone_id >= 0 )
	{
		int mbid = m_bone_ids[ AT.bone_id ];
		if( mbid >= 0 )
		{
			if( m_cachedMeshInst->IsSkinned() )
			{
				outwm = m_cachedMeshInst->skin_matrices[ mbid ] * outwm;
			}
			outwm = m_cachedMesh->m_bones[ mbid ].skinOffset * outwm;
		}
	}
	outwm = Mat4::CreateRotationFromQuat( AT.rotation ) *
		Mat4::CreateTranslation( AT.position ) * outwm;
	return true;
}


void AnimCharInst::RaycastAll( const Vec3& from, const Vec3& to, SceneRaycastCallback* cb, SGRX_MeshInstance* cbmi )
{
	UNUSED( cbmi ); // always use own mesh instance
	if( !m_cachedMesh )
		return;
	for( size_t i = 0; i < animChar->bones.size(); ++i )
	{
		Mat4 bxf, inv;
		AnimCharacter::BoneInfo& BI = animChar->bones[ i ];
		if( BI.hitbox.multiplier == 0 )
			continue;
		_GetHitboxMatrix( i, bxf );
		if( bxf.InvertTo( inv ) )
		{
			Vec3 p0 = inv.TransformPos( from );
			Vec3 p1 = inv.TransformPos( to );
			float dst[1];
			if( SegmentAABBIntersect2( p0, p1, -BI.hitbox.extents, BI.hitbox.extents, dst ) )
			{
				Vec3 N = ( from - to ).Normalized();
				SceneRaycastInfo srci = { dst[0], N, 0, 0, -1, -1, _GetMeshBoneID( i ), m_cachedMeshInst };
				cb->AddResult( &srci );
			}
		}
	}
}

void AnimCharInst::MRC_DebugDraw( SGRX_MeshInstance* mi )
{
	UNUSED( mi ); // always use own mesh instance
	if( !m_cachedMesh )
		return;
	
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Reset().Col( 0.1f, 0.5f, 0.9f );
	for( size_t i = 0; i < animChar->bones.size(); ++i )
	{
		Mat4 bxf;
		AnimCharacter::BoneInfo& BI = animChar->bones[ i ];
		if( BI.hitbox.multiplier == 0 )
			continue;
		_GetHitboxMatrix( i, bxf );
		br.AABB( -BI.hitbox.extents, BI.hitbox.extents, bxf * m_cachedMeshInst->matrix );
	}
}

uint16_t AnimCharInst::MEGetID( StringView name ) const
{
	return animChar->MEGetID( name );
}

double AnimCharInst::MEGetValue( uint16_t i ) const
{
	if( size_t(i) < m_values.size() )
		return m_values[ i ];
	return 0;
}

AnimCharHandle GR_GetAnimChar( const StringView& name )
{
	LOG_FUNCTION_ARG( name );
	
	if( !name )
		return NULL;
	AnimCharHandle out = g_AnimChars->getcopy( name );
	if( out )
		return out;
	
	double t0 = sgrx_hqtime();
	out = new AnimCharacter;
	if( !out->Load( name ) )
	{
		LOG_ERROR << LOG_DATE << "  Failed to load animated character: " << name;
		return NULL;
	}
	out->m_key = name;
	g_AnimChars->set( out->m_key, out );
	if( VERBOSE )
		LOG << "Loaded animated character " << name
		<< " (time=" << ( sgrx_hqtime() - t0 ) << ")";
	return out;
}



SGRX_LensFlare::SGRX_LensFlare() :
	m_fadeSpeed( 5 ),
	m_fadeState( 0 )
{
	TestData();
}

void SGRX_LensFlare::TestData()
{
	{Part p = { "textures/fx/flare_base.stx", 0.5f, 1, V4( 1 ) };
	p.ReloadTexture(); m_parts.push_back( p );}
	{Part p = { "textures/fx/flare_glow.stx", 0.15f, 0.6f, V4( 0.9f, 0.8f, 0.7f, 0.3f ) };
	p.ReloadTexture(); m_parts.push_back( p );}
	{Part p = { "textures/fx/flare_ring_2.stx", 0.35f, 0.3f, V4( 0.7f, 0.8f, 0.9f, 0.5f ) };
	p.ReloadTexture(); m_parts.push_back( p );}
	{Part p = { "textures/fx/flare_ring.stx", 0.25f, -0.4f, V4( 0.9f, 0.8f, 0.9f, 0.4f ) };
	p.ReloadTexture(); m_parts.push_back( p );}
	{Part p = { "textures/fx/flare_ring_2.stx", 0.3f, -0.8f, V4( 0.9f, 0.8f, 0.7f, 0.2f ) };
	p.ReloadTexture(); m_parts.push_back( p );}
}

bool SGRX_LensFlare::Load( StringView filename )
{
	return false;
}

bool SGRX_LensFlare::Save( StringView filename )
{
	return false;
}

void SGRX_LensFlare::Draw( float deltaTime, SceneHandle scene, Vec2 viewport, Vec3 pos, Vec3 lightColor )
{
	Vec2 screenpos;
	const Vec3& cp = scene->camera.position;
	const Vec3& cd = scene->camera.direction;
	// behind camera
	bool visible = lightColor != V3(0);
	if( visible )
		visible = Vec3Dot( cd, pos ) > Vec3Dot( cd, cp );
	// outside view
	if( visible )
	{
		screenpos = scene->camera.WorldToScreen( pos ).ToVec2();
		visible = screenpos.x >= 0 &&
			screenpos.x <= 1 &&
			screenpos.y >= 0 &&
			screenpos.y <= 1;
	}
	// occluded
	if( visible )
		visible = scene->RaycastAny( scene->camera.position, pos ) == false;
	
	m_fadeState += m_fadeSpeed * deltaTime * ( visible ? 1 : -1 );
	m_fadeState = TCLAMP( m_fadeState, 0.0f, 1.0f );
	
	if( m_fadeState <= 0 )
		return;
	
	BatchRenderer& br = GR2D_GetBatchRenderer();
	screenpos *= viewport;
	Vec2 centerpos = viewport * 0.5f;
	float fullsize = TMIN( viewport.x, viewport.y );
	float alpha = 1 - TCLAMP( ( screenpos - centerpos ).Length() / ( fullsize * 0.5f ), 0.0f, 1.0f );
	for( size_t i = 0; i < m_parts.size(); ++i )
	{
		br.Reset();
		const Part& P = m_parts[ i ];
		Vec2 fpos = TLERP( centerpos, screenpos, P.pos );
		float fsz = fullsize * P.size;
		br.Col( P.color * V4( lightColor, m_fadeState * alpha ) );
		br.SetTexture( P.texture );
		br.Box( fpos.x, fpos.y, fsz, fsz );
	}
}


