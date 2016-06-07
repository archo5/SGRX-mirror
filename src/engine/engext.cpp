

#include "engext.hpp"



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
				if( ME_OPCODE_TYPE( op.type ) == OT_VAR && op.op1 >= i )
					op.op1--;
				else if( op.type & ME_OPCODE_OP1VARBL && op.op1 >= i )
					op.op1--;
				else if( op.type & ME_OPCODE_OP2VARBL && op.op2 >= i )
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

void AnimRagdoll::Initialize( AnimCharacter* chinfo )
{
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
	
	if( m_pose.size() != chinfo->bones.size() )
	{
		// ragdoll not used
		return;
	}
	
	for( size_t bid = 0; bid < chinfo->bones.size(); ++bid )
	{
		AnimCharacter::BoneInfo& BI = chinfo->bones[ bid ];
		
		Body* TB = &m_bones[ BI.bone_id ];
		
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
		AnimCharacter::BoneInfo& BI = chinfo->bones[ bid ];
		
		if( BI.joint.parent_id >= 0 && BI.joint.type != AnimCharacter::JointType_None )
		{
			AnimCharacter::BoneInfo& PBI = chinfo->bones[ BI.joint.parent_id ];
			Mat4 jsm, jpm;
			if( m_bones[ BI.bone_id ].bodyHandle &&
				m_bones[ PBI.bone_id ].bodyHandle &&
				chinfo->GetJointFrameMatrices( bid, jsm, jpm ) )
			{
				if( BI.joint.type == AnimCharacter::JointType_Hinge )
				{
					SGRX_PhyHingeJointInfo jinfo;
					jinfo.enabled = false;
					jinfo.bodyA = m_bones[ BI.bone_id ].bodyHandle;
					jinfo.bodyB = m_bones[ PBI.bone_id ].bodyHandle;
					jinfo.frameA = jsm;
					jinfo.frameB = jpm;
					m_bones[ BI.bone_id ].jointHandle =
						m_phyWorld->CreateHingeJoint( jinfo );
				}
				else if( BI.joint.type == AnimCharacter::JointType_ConeTwist )
				{
					SGRX_PhyConeTwistJointInfo jinfo;
					jinfo.enabled = false;
					jinfo.bodyA = m_bones[ BI.bone_id ].bodyHandle;
					jinfo.bodyB = m_bones[ PBI.bone_id ].bodyHandle;
					jinfo.frameA = jsm;
					jinfo.frameB = jpm;
					jinfo.coneLimitX = DEG2RAD( BI.joint.turn_limit_1 );
					jinfo.coneLimitY = DEG2RAD( BI.joint.turn_limit_2 );
					jinfo.twistLimit = DEG2RAD( BI.joint.twist_limit );
					m_bones[ BI.bone_id ].jointHandle =
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
	for( size_t i = 0; i < m_pose.size(); ++i )
		m_pose[ i ].fq = m_enabled && m_bones[ i ].bodyHandle;
	
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

Animator* AnimCharacter::RagdollNode::GetAnimator( AnimCharacter* ch )
{
	return &ch->m_anRagdoll;
}

Animator* AnimCharacter::LayersNode::GetAnimator( AnimCharacter* ch )
{
	return &ch->m_anLayers;
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



AnimCharacter::AnimCharacter( SceneHandle sh, PhyWorldHandle phyWorld ) :
	m_scene( sh ),
	m_frameID( 0 ),
	m_anRagdoll( phyWorld )
{
	ASSERT( sh && "scene handle must be valid" );
	
	m_cachedMeshInst = m_scene->CreateMeshInstance();
	m_cachedMeshInst->raycastOverride = this;
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
	
	_OnRenderUpdate();
	m_cachedMeshInst->raycastOverride = m_cachedMesh->m_numBones ? this : NULL;
	return true;
}

bool AnimCharacter::Save( const StringView& sv )
{
	ByteArray ba;
	ByteWriter bw( &ba );
	Serialize( bw );
	return FS_SaveBinaryFile( sv, ba.data(), ba.size() );
}

void AnimCharacter::_OnRenderUpdate()
{
	m_cachedMesh = GR_GetMesh( mesh );
	m_cachedMeshInst->SetMesh( m_cachedMesh );
	m_cachedMeshInst->skin_matrices.resize( m_cachedMesh.GetBoneCount() );
	RecalcBoneIDs();
	_Prepare();
	m_anRagdoll.Initialize( this );
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

void AnimCharacter::_Prepare()
{
	// count animators
	int num_animators = 3; // ragdoll, layers, end
	int num_bones = m_cachedMesh.GetBoneCount();
	for( size_t i = 0; i < nodes.size(); ++i )
	{
		if( nodes[ i ]->OwnsAnimator() )
			num_animators++;
	}
	
	// allocate frames
	m_node_frames.resize( num_animators * num_bones );
	for( size_t i = 0; i < m_node_frames.size(); ++i )
		m_node_frames[ i ].Reset();
	
	// equip animators
	int at = 0;
	_EquipAnimator( &m_anRagdoll, at++ );
	_EquipAnimator( &m_anLayers, at++ );
	_EquipAnimator( &m_anEnd, at++ );
	for( size_t i = 0; i < nodes.size(); ++i )
	{
		Animator* anim = nodes[ i ]->GetAnimator( this );
		if( anim && nodes[ i ]->OwnsAnimator() )
			_EquipAnimator( anim, at++ );
	}
	
	// rehash & link
	_RehashNodes();
	for( size_t i = 0; i < nodes.size(); ++i )
	{
		for( int l = 0; l < nodes[ i ]->GetInputLinkCount(); ++l )
		{
			const SGRX_GUID* pguid = nodes[ i ]->GetInputLink( l );
			Animator** panim = nodes[ i ]->GetInputSource( l );
			Node* node = m_node_map.getcopy( *pguid );
			if( node )
				*panim = node->GetAnimator( this );
		}
	}
	m_anEnd.animSource = output_node ? output_node->GetAnimator( this ) : NULL;
	
	// additional work
	for( size_t i = 0; i < m_anLayers.m_pose.size(); ++i )
		m_anLayers.m_pose[ i ].fq = 1;
}

void AnimCharacter::_EquipAnimator( Animator* anim, int which )
{
	int num_bones = m_cachedMesh.GetBoneCount();
	int at = which * num_bones;
	anim->m_mesh = m_cachedMesh;
	anim->m_pose = ArrayView<AnimTrackFrame>( m_node_frames ).part( at, num_bones );
	anim->Prepare();
}

void AnimCharacter::SetTransform( const Mat4& mtx )
{
	if( m_cachedMeshInst )
		m_cachedMeshInst->matrix = mtx;
}

void AnimCharacter::FixedTick( float deltaTime )
{
	for( size_t i = 0; i < nodes.size(); ++i )
		nodes[ i ]->Advance( deltaTime, this );
	AnimInfo info = { ++m_frameID, m_cachedMeshInst->matrix };
	m_anEnd.Advance( deltaTime, &info );
	m_anRagdoll.AdvanceTransforms( &m_anEnd );
}

void AnimCharacter::PreRender( float blendFactor )
{
	m_anEnd.Interpolate( blendFactor );
	GR_ApplyAnimator( &m_anEnd, m_cachedMeshInst );
}

void AnimCharacter::RecalcLayerState()
{
	if( m_cachedMesh == NULL )
		return;
	
	for( size_t i = 0; i < m_anLayers.m_pose.size(); ++i )
		m_anLayers.m_pose[ i ].Reset();
	for( size_t i = 0; i < layers.size(); ++i )
	{
		Layer& L = layers[ i ];
		for( size_t j = 0; j < L.transforms.size(); ++j )
		{
			LayerTransform& LT = L.transforms[ j ];
			if( LT.bone_id < 0 )
				continue;
			switch( LT.type )
			{
			case TransformType_UndoParent:
				{
					int parent_id = m_cachedMesh->m_bones[ LT.bone_id ].parent_id;
					if( parent_id >= 0 )
					{
						m_anLayers.m_pose[ LT.bone_id ].pos -= m_anLayers.m_pose[ parent_id ].pos;
						m_anLayers.m_pose[ LT.bone_id ].rot =
							m_anLayers.m_pose[ parent_id ].rot.Inverted() * m_anLayers.m_pose[ LT.bone_id ].rot;
					}
				}
				break;
			case TransformType_Move:
				m_anLayers.m_pose[ LT.bone_id ].pos += LT.posaxis * ( LT.base + L.amount );
				break;
			case TransformType_Rotate:
				m_anLayers.m_pose[ LT.bone_id ].rot = m_anLayers.m_pose[ LT.bone_id ].rot
					* Quat::CreateAxisAngle( LT.posaxis.Normalized(), DEG2RAD( LT.angle ) * ( LT.base + L.amount ) );
				break;
			}
		}
	}
}

void AnimCharacter::EnablePhysics()
{
	m_anRagdoll.EnablePhysics( m_cachedMeshInst->matrix );
}

void AnimCharacter::DisablePhysics()
{
	m_anRagdoll.DisablePhysics();
}

void AnimCharacter::WakeUp()
{
	m_anRagdoll.WakeUp();
}

int AnimCharacter::_FindBone( const StringView& name )
{
	if( !m_cachedMesh )
		return -1;
	int bid = 0;
	for( ; bid < m_cachedMesh->m_numBones; ++bid )
	{
		if( m_cachedMesh->m_bones[ bid ].name == name )
			break;
	}
	return bid < m_cachedMesh->m_numBones ? bid : -1;
}

int AnimCharacter::FindParentBone( int which )
{
	if( m_cachedMesh == NULL )
		return -1;
	if( which < 0 || which >= (int) bones.size() )
		return -1;
	
	int parent_id = m_cachedMesh->m_bones[ bones[ which ].bone_id ].parent_id;
	for( size_t i = 0; i < bones.size(); ++i )
	{
		if( bones[ i ].bone_id == parent_id )
			return i;
	}
	return -1;
}

void AnimCharacter::RecalcBoneIDs()
{
	for( size_t i = 0; i < bones.size(); ++i )
	{
		BoneInfo& BI = bones[ i ];
		BI.bone_id = _FindBone( BI.name );
		BI.joint.parent_id = _FindBone( BI.joint.parent_name );
	}
	for( size_t i = 0; i < attachments.size(); ++i )
	{
		Attachment& AT = attachments[ i ];
		AT.bone_id = _FindBone( AT.bone );
	}
	for( size_t i = 0; i < layers.size(); ++i )
	{
		Layer& LY = layers[ i ];
		for( size_t j = 0; j < LY.transforms.size(); ++j )
		{
			LayerTransform& LT = LY.transforms[ j ];
			LT.bone_id = _FindBone( LT.bone );
		}
	}
}

bool AnimCharacter::GetBodyMatrix( int which, Mat4& outwm )
{
	if( !m_cachedMesh )
		return false;
	if( which < 0 || which >= (int) bones.size() )
		return false;
	BoneInfo& BI = bones[ which ];
	
	outwm = m_cachedMeshInst->matrix;
	if( BI.bone_id >= 0 )
	{
		if( m_cachedMeshInst->IsSkinned() )
		{
			outwm = m_cachedMeshInst->skin_matrices[ BI.bone_id ] * outwm;
		}
		outwm = m_cachedMesh->m_bones[ BI.bone_id ].skinOffset * outwm;
	}
	outwm = Mat4::CreateRotationFromQuat( BI.body.rotation ) *
		Mat4::CreateTranslation( BI.body.position ) * outwm;
	return true;
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

bool AnimCharacter::GetJointMatrix( int which, bool parent, Mat4& outwm )
{
	if( !m_cachedMesh )
		return false;
	if( which < 0 || which >= (int) bones.size() )
		return false;
	BoneInfo& BI = bones[ which ];
	int bid = BI.bone_id;
	if( parent )
	{
		int pb = BI.joint.parent_id;
		if( pb < 0 || pb >= (int) bones.size() )
			return false;
		bid = bones[ pb ].bone_id;
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

void AnimCharacter::_GetHitboxMatrix( int which, Mat4& outwm )
{
	BoneInfo& BI = bones[ which ];
	outwm = Mat4::CreateRotationFromQuat( BI.hitbox.rotation ) *
		Mat4::CreateTranslation( BI.hitbox.position );
	if( BI.bone_id >= 0 )
	{
		outwm = outwm * m_cachedMesh->m_bones[ BI.bone_id ].skinOffset;
		if( m_cachedMeshInst->IsSkinned() )
		{
			outwm = outwm * m_cachedMeshInst->skin_matrices[ BI.bone_id ];
		}
	}
}

bool AnimCharacter::GetHitboxOBB( int which, Mat4& outwm, Vec3& outext )
{
	if( !m_cachedMesh )
		return false;
	if( which < 0 || which >= (int) bones.size() )
		return false;
	BoneInfo& BI = bones[ which ];
	if( BI.hitbox.multiplier == 0 )
		return false; // a way to disable it
	
	_GetHitboxMatrix( which, outwm );
	outwm = outwm * m_cachedMeshInst->matrix;
	outext = BI.hitbox.extents;
	return true;
}

bool AnimCharacter::GetAttachmentMatrix( int which, Mat4& outwm, bool worldspace ) const
{
	outwm = worldspace ? m_cachedMeshInst->matrix : Mat4::Identity;
	if( !m_cachedMesh )
		return false;
	if( which < 0 || which >= (int) attachments.size() )
		return false;
	const Attachment& AT = attachments[ which ];
	
	if( AT.bone_id >= 0 )
	{
		if( m_cachedMeshInst->IsSkinned() )
		{
			outwm = m_cachedMeshInst->skin_matrices[ AT.bone_id ] * outwm;
		}
		outwm = m_cachedMesh->m_bones[ AT.bone_id ].skinOffset * outwm;
	}
	outwm = Mat4::CreateRotationFromQuat( AT.rotation ) *
		Mat4::CreateTranslation( AT.position ) * outwm;
	return true;
}

bool AnimCharacter::ApplyMask( const StringView& name, AnimMask* tgt )
{
	if( !m_cachedMesh )
		return false;
	
	for( size_t i = 0; i < masks.size(); ++i )
	{
		Mask& M = masks[ i ];
		if( M.name != name )
			continue;
		
		Array< float >& factors = tgt->blendFactors;
		GR_ClearFactors( factors, 0 );
		for( size_t j = 0; j < M.cmds.size(); ++j )
		{
			MaskCmd& MC = M.cmds[ j ];
			GR_SetFactors( factors, m_cachedMesh, MC.bone, MC.weight, MC.children );
		}
		return true;
	}
	return false;
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

void AnimCharacter::SortEnsureAttachments( const StringView* atchnames, int count )
{
	for( int i = 0; i < count; ++i )
	{
		int aid = -1;
		for( size_t j = i; j < attachments.size(); ++j )
		{
			if( attachments[ i ].name == atchnames[ i ] )
			{
				aid = j;
				break;
			}
		}
		if( aid == i )
			continue; // at the right place already
		if( aid != -1 )
		{
			TMEMSWAP( attachments[ i ], attachments[ aid ] );
		}
		else
		{
			attachments.insert( i, Attachment() );
			attachments[ i ].name = atchnames[ i ];
		}
	}
}

void AnimCharacter::RaycastAll( const Vec3& from, const Vec3& to, SceneRaycastCallback* cb, SGRX_MeshInstance* cbmi )
{
	UNUSED( cbmi ); // always use own mesh instance
	if( !m_cachedMesh )
		return;
	for( size_t i = 0; i < bones.size(); ++i )
	{
		Mat4 bxf, inv;
		BoneInfo& BI = bones[ i ];
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
				SceneRaycastInfo srci = { dst[0], N, 0, 0, -1, -1, BI.bone_id, m_cachedMeshInst };
				cb->AddResult( &srci );
			}
		}
	}
}

void AnimCharacter::MRC_DebugDraw( SGRX_MeshInstance* mi )
{
	UNUSED( mi ); // always use own mesh instance
	if( !m_cachedMesh )
		return;
	
	BatchRenderer& br = GR2D_GetBatchRenderer();
	br.Reset().Col( 0.1f, 0.5f, 0.9f );
	for( size_t i = 0; i < bones.size(); ++i )
	{
		Mat4 bxf;
		BoneInfo& BI = bones[ i ];
		if( BI.hitbox.multiplier == 0 )
			continue;
		_GetHitboxMatrix( i, bxf );
		br.AABB( -BI.hitbox.extents, BI.hitbox.extents, bxf * m_cachedMeshInst->matrix );
	}
}

uint16_t AnimCharacter::MEGetID( StringView name ) const
{
	for( size_t i = 0; i < variables.size(); ++i )
	{
		if( variables[ i ]->name == name )
			return (uint16_t) i;
	}
	return ME_OPERAND_NONE;
}

double AnimCharacter::MEGetValue( uint16_t i ) const
{
	if( size_t(i) >= variables.size() )
		return 0;
	return variables[ i ]->value;
}


void AnimCharacter::_RehashNodes()
{
	m_node_map.clear();
	for( size_t i = 0; i < nodes.size(); ++i )
	{
		m_node_map.set( nodes[ i ]->guid, nodes[ i ] );
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

void AnimCharacter::_SetVar( StringView name, float val )
{
	uint16_t* id = m_variable_index.getptr( name );
	if( !id || size_t(*id) >= variables.size() )
		return;
	variables[ *id ]->value = val;
}



SGRX_DecalSystem::SGRX_DecalSystem() : m_lightSampler(NULL), m_vbSize(0)
{
}

SGRX_DecalSystem::~SGRX_DecalSystem()
{
	Free();
}

void SGRX_DecalSystem::Init( SceneHandle scene, TextureHandle texDecal, TextureHandle texFalloff )
{
	m_vertexDecl = GR_GetVertexDecl( SGRX_VDECL_DECAL );
	m_mesh = GR_CreateMesh();
	
	m_meshInst = scene->CreateMeshInstance();
	m_meshInst->SetMesh( m_mesh );
	SGRX_Material mtl;
	mtl.flags = SGRX_MtlFlag_Decal | SGRX_MtlFlag_VCol;
	mtl.blendMode = SGRX_MtlBlend_Basic;
	mtl.shader = "decal";
	mtl.textures[0] = texDecal;
	mtl.textures[1] = texFalloff;
	m_meshInst->materials.assign( &mtl, 1 );
	m_meshInst->SetMITexture( 0, GR_GetTexture( "textures/white.png" ) );
	m_meshInst->SetLightingMode( SGRX_LM_Decal );
}

void SGRX_DecalSystem::Free()
{
	ClearAllDecals();
	m_mesh = NULL;
	m_meshInst = NULL;
	m_vbSize = 0;
}

void SGRX_DecalSystem::SetSize( uint32_t vbSize )
{
	m_vbSize = vbSize;
}

void SGRX_DecalSystem::SetDynamic( bool dynamic )
{
	m_meshInst->SetLightingMode( dynamic ? SGRX_LM_Dynamic : SGRX_LM_Decal );
}

void SGRX_DecalSystem::Upload()
{
	if( m_mesh == NULL )
		return;
	
	// cut excess decals
	size_t vbcutsize = 0, vbsize = m_vertexData.size(), ibcutsize = 0, cutcount = 0;
	while( vbsize > m_vbSize + vbcutsize )
	{
		vbcutsize += m_decals[ cutcount++ ];
		ibcutsize += m_decals[ cutcount++ ];
	}
	if( cutcount )
	{
		m_vertexData.erase( 0, vbcutsize );
		m_indexData.erase( 0, ibcutsize );
		m_decals.erase( 0, cutcount );
		uint32_t iboff = vbcutsize / sizeof(SGRX_Vertex_Decal);
		for( size_t i = 0; i < m_indexData.size(); ++i )
		{
			m_indexData[ i ] -= iboff;
		}
	}
	
	// apply data
	if( m_vertexData.size() )
	{
		m_mesh->SetVertexData( m_vertexData.data(), m_vertexData.size_bytes(), m_vertexDecl );
		m_mesh->SetIndexData( m_indexData.data(), m_indexData.size_bytes(), true );
	}
	SGRX_MeshPart mp = { 0, m_vertexData.size() / sizeof(SGRX_Vertex_Decal), 0, m_indexData.size() };
	m_mesh->SetPartData( &mp, 1 );
}

void SGRX_DecalSystem::AddDecal( const DecalProjectionInfo& projInfo, SGRX_IMesh* targetMesh, const Mat4& worldMatrix )
{
	float inv_zn2zf;
	Mat4 vpmtx;
	_GenDecalMatrix( projInfo, &vpmtx, &inv_zn2zf );
	uint32_t color = Vec3ToCol32( m_lightSampler ?
		m_lightSampler->SampleLight( projInfo.pos ) * 0.125f : V3(0.125f) );
	
	size_t origvbsize = m_vertexData.size(), origibsize = m_indexData.size();
	targetMesh->Clip( worldMatrix, vpmtx, m_vertexData, true, inv_zn2zf, color );
	if( m_vertexData.size() > origvbsize )
	{
		_ScaleDecalTexcoords( projInfo, origvbsize );
		_InvTransformDecals( origvbsize );
		SGRX_DoIndexTriangleMeshVertices( m_indexData, m_vertexData, origvbsize, sizeof(SGRX_Vertex_Decal) );
		m_decals.push_back( m_vertexData.size() - origvbsize );
		m_decals.push_back( m_indexData.size() - origibsize );
	}
}

void SGRX_DecalSystem::AddDecal( const DecalProjectionInfo& projInfo, SGRX_IMesh* targetMesh, int partID, const Mat4& worldMatrix )
{
	float inv_zn2zf;
	Mat4 vpmtx;
	_GenDecalMatrix( projInfo, &vpmtx, &inv_zn2zf );
	uint32_t color = Vec3ToCol32( m_lightSampler ?
		m_lightSampler->SampleLight( projInfo.pos ) * 0.125f : V3(0.125f) );
	
	size_t origvbsize = m_vertexData.size(), origibsize = m_indexData.size();
	targetMesh->Clip( worldMatrix, vpmtx, m_vertexData, true, inv_zn2zf, color, partID, 1 );
	if( m_vertexData.size() > origvbsize )
	{
		_ScaleDecalTexcoords( projInfo, origvbsize );
		_InvTransformDecals( origvbsize );
		SGRX_DoIndexTriangleMeshVertices( m_indexData, m_vertexData, origvbsize, sizeof(SGRX_Vertex_Decal) );
		m_decals.push_back( m_vertexData.size() - origvbsize );
		m_decals.push_back( m_indexData.size() - origibsize );
	}
}

void SGRX_DecalSystem::ClearAllDecals()
{
	m_vertexData.clear();
	m_indexData.clear();
	m_decals.clear();
}

void SGRX_DecalSystem::GenerateCamera( const DecalProjectionInfo& projInfo, SGRX_Camera* out )
{
	// TODO for now...
	ASSERT( projInfo.perspective );
	
	const DecalMapPartInfo& DMPI = projInfo.decalInfo;
	float dist = DMPI.size.z * projInfo.distanceScale;
	
	out->position = projInfo.pos - projInfo.dir * projInfo.pushBack * dist;
	out->direction = projInfo.dir;
	out->updir = projInfo.up;
	out->angle = projInfo.fovAngleDeg;
	out->aspect = projInfo.aspectMult;
	out->aamix = projInfo.aamix;
	out->znear = dist * 0.001f;
	out->zfar = dist;
	
	out->UpdateMatrices();
}

void SGRX_DecalSystem::_ScaleDecalTexcoords( const DecalProjectionInfo& projInfo, size_t vbfrom )
{
	const DecalMapPartInfo& DMPI = projInfo.decalInfo;
	
	SGRX_CAST( SGRX_Vertex_Decal*, vdata, m_vertexData.data() );
	SGRX_Vertex_Decal* vdend = vdata + m_vertexData.size() / sizeof(SGRX_Vertex_Decal);
	vdata += vbfrom / sizeof(SGRX_Vertex_Decal);
	while( vdata < vdend )
	{
		vdata->texcoord.x = TLERP( DMPI.bbox.x, DMPI.bbox.z, vdata->texcoord.x );
		vdata->texcoord.y = TLERP( DMPI.bbox.y, DMPI.bbox.w, vdata->texcoord.y );
		vdata++;
	}
}

void SGRX_DecalSystem::_InvTransformDecals( size_t vbfrom )
{
	if( m_meshInst->GetLightingMode() == SGRX_LM_Decal )
		return;
	
	Mat4 inv = Mat4::Identity;
	m_meshInst->matrix.InvertTo( inv );
	
	SGRX_CAST( SGRX_Vertex_Decal*, vdata, m_vertexData.data() );
	SGRX_Vertex_Decal* vdend = vdata + m_vertexData.size() / sizeof(SGRX_Vertex_Decal);
	vdata += vbfrom / sizeof(SGRX_Vertex_Decal);
	while( vdata < vdend )
	{
		vdata->position = inv.TransformPos( vdata->position );
		vdata->normal = inv.TransformNormal( vdata->normal );
		vdata++;
	}
}

void SGRX_DecalSystem::_GenDecalMatrix( const DecalProjectionInfo& projInfo, Mat4* outVPM, float* out_invzn2zf )
{
	const DecalMapPartInfo& DMPI = projInfo.decalInfo;
	
	float znear = 0, dist = DMPI.size.z * projInfo.distanceScale;
	Mat4 projMtx, viewMtx = Mat4::CreateLookAt(
		projInfo.pos - projInfo.dir * projInfo.pushBack * dist,
		projInfo.dir, projInfo.up );
	if( projInfo.perspective )
	{
		float aspect = DMPI.size.x / DMPI.size.y * projInfo.aspectMult;
		znear = dist * 0.001f;
		projMtx = Mat4::CreatePerspective( projInfo.fovAngleDeg, aspect, projInfo.aamix, znear, dist );
		*out_invzn2zf = safe_fdiv( 1, dist - znear );
	}
	else
	{
		Vec2 psz = DMPI.size.ToVec2() * 0.5f * projInfo.orthoScale;
		projMtx = Mat4::CreateOrtho( V3( -psz.x, -psz.y, 0 ), V3( psz.x, psz.y, DMPI.size.z * projInfo.distanceScale ) );
		*out_invzn2zf = 0;
	}
	*outVPM = viewMtx * projMtx;
}



SGRX_HelpTextRenderer::SGRX_HelpTextRenderer() :
	fontSize(16), centerPos(V2(100,100)), lineHeightFactor(1.2f),
	buttonTexBorder(40), buttonBorder(4), opacity(1),
	m_lineCount(0), m_curLine(0), m_curColor(V4(1))
{
	m_curFont.sizeFactor = 1;
}

void SGRX_HelpTextRenderer::RenderText( StringView text )
{
	m_curLine = 0;
	m_lineCount = text.count( "\n" );
	SetColor( "" );
	SetFont( "" );
	m_strings.clear();
	m_textCache.clear();
	
	StringView start = text, it = text;
	while( it.size() )
	{
		char c = it.ch();
		if( c == '#' )
		{
			AddText( start, it );
			it.skip( 1 );
			char c2 = it.ch();
			if( c2 == '#' )
			{
				start = it;
				it.skip( 1 ); // skip one #, include another
				continue;
			}
			else if( c2 == 'c' || c2 == 'C' ) // color
			{
				it.skip( 1 );
				if( it.ch() != '(' )
				{
					LOG_WARNING << "RenderText: unexpected character at " << (it.data() - text.data());
					continue;
				}
				it.skip( 1 );
				StringView colorname = it.until( ")" );
				SetColor( colorname );
				it.skip( colorname.size() + 1 );
				start = it;
				continue;
			}
			else if( c2 == 'f' || c2 == 'F' ) // font
			{
				it.skip( 1 );
				if( it.ch() != '(' )
				{
					LOG_WARNING << "RenderText: unexpected character at " << (it.data() - text.data());
					continue;
				}
				it.skip( 1 );
				StringView fontname = it.until( ")" );
				SetFont( fontname );
				it.skip( fontname.size() + 1 );
				start = it;
				continue;
			}
			else if( c2 == 'a' || c2 == 'A' ) // action
			{
				it.skip( 1 );
				if( it.ch() != '(' )
				{
					LOG_WARNING << "RenderText: unexpected character at " << (it.data() - text.data());
					continue;
				}
				it.skip( 1 );
				StringView actionname = it.until( ")" );
				AddActionInputText( actionname );
				it.skip( actionname.size() + 1 );
				start = it;
				continue;
			}
		}
		else if( c == '\n' )
		{
			AddText( start, it );
			m_curLine++;
			it.skip( 1 );
			start = it;
		}
		it.skip( 1 );
	}
	if( start.size() )
		AddText( start );
	
	// calculate h-positions
	size_t si = 0;
	int curline = 0;
	int totalWidth = 0;
	for( size_t i = 0; i <= m_textCache.size(); ++i )
	{
		if( i == m_textCache.size() || curline != m_textCache[ i ].lineNum )
		{
			int ho = 0;
			for( ; si < i; ++si )
			{
				m_textCache[ si ].pos.x += ho - totalWidth / 2;
				ho += m_textCache[ si ].width;
			}
			
			if( i == m_textCache.size() )
				break;
			
			curline = m_textCache[ i ].lineNum;
			totalWidth = 0;
		}
		totalWidth += m_textCache[ i ].width;
	}
	
	// draw text items
	for( size_t i = 0; i < m_textCache.size(); ++i )
		DrawTextItem( m_textCache[ i ] );
	
	m_textCache.clear();
}

void SGRX_HelpTextRenderer::AddInputText( ActionInput input )
{
	StringView text = Game_GetInputName( input );
	AddText( text );
	int pad = buttonBorder;
	m_textCache.last().padding = pad;
	m_textCache.last().width += pad * 2;
	m_textCache.last().button = true;
}

void SGRX_HelpTextRenderer::AddActionInputText( StringView action )
{
	InputState* cmd = Game_FindAction( action );
	ActionInput inputs[2];
	int num = Game_GetActionBindings( cmd, inputs, 2 );
	for( int i = 0; i < num; ++i )
	{
		if( i != 0 )
			AddText( "/" );
		AddInputText( inputs[ i ] );
	}
}

void SGRX_HelpTextRenderer::DrawTextItem( Text& item )
{
	BatchRenderer& br = GR2D_GetBatchRenderer();
	
	Vec4 c = item.color;
	Vec2 pos = V2( round( item.pos.x ), round( item.pos.y ) );
	if( item.button )
	{
		int hsize = item.fontSize / 2 + item.padding;
		Vec4 rect = V4( pos.x, pos.y - hsize, pos.x + item.width, pos.y + hsize );
		Vec2 texoff = buttonTex.GetInvSize() * buttonTexBorder;
		Vec4 texbdr = V4( texoff.x, texoff.y, texoff.x, texoff.y );
		br.Reset().Col( 1, opacity )
		  .SetTexture( buttonTex ).Button( rect, V4(buttonBorder), texbdr );
		c = V4( V3(0.1f), 1 );
	}
	
	GR2D_SetColor( c.x, c.y, c.z, c.w * opacity );
	GR2D_SetFont( item.font, item.fontSize );
	GR2D_DrawTextLine( item.pos.x + item.padding, item.pos.y, GetText( item ), HALIGN_LEFT, VALIGN_CENTER );
}

void SGRX_HelpTextRenderer::SetColor( StringView name )
{
	Vec4* col = namedColors.getptr( name );
	m_curColor = col ? *col : V4(1);
}

void SGRX_HelpTextRenderer::SetFont( StringView name )
{
	FontInfo* font = namedFonts.getptr( name );
	if( font )
		m_curFont = *font;
	else
	{
		FontInfo dummy = { "", 1 };
		m_curFont = dummy;
	}
}

void SGRX_HelpTextRenderer::AddText( StringView text )
{
	Text T =
	{
		m_strings.size(),
		text.size(),
		m_curFont.name,
		m_curColor,
		centerPos,
		fontSize * m_curFont.sizeFactor,
		0,
		m_curLine,
		false
	};
	m_strings.append( text );
	T.pos.y += ( m_curLine - ( m_lineCount - 1 ) * 0.5f ) * fontSize * lineHeightFactor;
	GR2D_SetFont( T.font, T.fontSize );
	T.width = GR2D_GetTextLength( text );
	m_textCache.push_back( T );
}



