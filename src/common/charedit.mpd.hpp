
#pragma once
#ifndef MPD_API_HPP
#  define MPD_API_HPP "mpd_api.hpp"
#endif
#include MPD_API_HPP

struct Vec2_MPD : struct_MPD<Vec2_MPD, Vec2 >
{
	enum PIDS
	{
		PID_x,
		PID_y,
	};

	static const char* name(){ return "Vec2"; }
	static const Vec2_MPD* inst(){ static const Vec2_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 2; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( Vec2 const*, int );
	static bool setprop( Vec2*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( Vec2 const*, const mpd_Variant& );
	static bool setindex( Vec2*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(Vec2) );
};

MPD_DUMPDATA_WRAPPER(Vec2, Vec2);
template<> struct mpd_MetaType<Vec2 > : Vec2_MPD {};
template<> struct mpd_MetaType<Vec2 const> : Vec2_MPD {};
template<> struct mpd_MetaType<Vec2_MPD> : Vec2_MPD {};

struct Vec3_MPD : struct_MPD<Vec3_MPD, Vec3 >
{
	enum PIDS
	{
		PID_x,
		PID_y,
		PID_z,
	};

	static const char* name(){ return "Vec3"; }
	static const Vec3_MPD* inst(){ static const Vec3_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 3; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( Vec3 const*, int );
	static bool setprop( Vec3*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( Vec3 const*, const mpd_Variant& );
	static bool setindex( Vec3*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(Vec3) );
};

MPD_DUMPDATA_WRAPPER(Vec3, Vec3);
template<> struct mpd_MetaType<Vec3 > : Vec3_MPD {};
template<> struct mpd_MetaType<Vec3 const> : Vec3_MPD {};
template<> struct mpd_MetaType<Vec3_MPD> : Vec3_MPD {};

struct String_MPD : struct_MPD<String_MPD, String >
{
	enum PIDS
	{
		PID_data,
		PID_size,
	};

	static const char* name(){ return "String"; }
	static const String_MPD* inst(){ static const String_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 2; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( String const*, int );
	static bool setprop( String*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ static const mpd_TypeInfo types[] = { { "int32_t", mpdt_Int32, 0 }, { "int8_t", mpdt_Int8, 0 } }; return types; }
	static mpd_Variant getindex( String const*, const mpd_Variant& );
	static bool setindex( String*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(String) );
};

MPD_DUMPDATA_WRAPPER(String, String);
template<> struct mpd_MetaType<String > : String_MPD {};
template<> struct mpd_MetaType<String const> : String_MPD {};
template<> struct mpd_MetaType<String_MPD> : String_MPD {};

struct Vec3Array_MPD : struct_MPD<Vec3Array_MPD, Array<Vec3> >
{
	enum PIDS
	{
		PID_size,
	};

	static const char* name(){ return "Vec3Array"; }
	static const Vec3Array_MPD* inst(){ static const Vec3Array_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 1; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( Array<Vec3> const*, int );
	static bool setprop( Array<Vec3>*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ static const mpd_TypeInfo types[] = { { "int32_t", mpdt_Int32, 0 }, { "Vec3", mpdt_Struct, Vec3_MPD::inst() } }; return types; }
	static mpd_Variant getindex( Array<Vec3> const*, const mpd_Variant& );
	static bool setindex( Array<Vec3>*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(Array<Vec3>) );
};

MPD_DUMPDATA_WRAPPER(Vec3Array, Array<Vec3>);
template<> struct mpd_MetaType<Array<Vec3> > : Vec3Array_MPD {};
template<> struct mpd_MetaType<Array<Vec3> const> : Vec3Array_MPD {};
template<> struct mpd_MetaType<Vec3Array_MPD> : Vec3Array_MPD {};

struct AnimCharacter_BodyType_MPD : struct_MPD<AnimCharacter_BodyType_MPD, AnimCharacter::BodyType >
{
	static const char* name(){ return "AnimCharacter_BodyType"; }
	static const AnimCharacter_BodyType_MPD* inst(){ static const AnimCharacter_BodyType_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 0; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( AnimCharacter::BodyType const*, int );
	static bool setprop( AnimCharacter::BodyType*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( AnimCharacter::BodyType const*, const mpd_Variant& );
	static bool setindex( AnimCharacter::BodyType*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 4; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(AnimCharacter::BodyType) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter_BodyType, AnimCharacter::BodyType);
template<> struct mpd_MetaType<AnimCharacter::BodyType > : AnimCharacter_BodyType_MPD {};
template<> struct mpd_MetaType<AnimCharacter::BodyType const> : AnimCharacter_BodyType_MPD {};
template<> struct mpd_MetaType<AnimCharacter_BodyType_MPD> : AnimCharacter_BodyType_MPD {};

struct AnimCharacter_JointType_MPD : struct_MPD<AnimCharacter_JointType_MPD, AnimCharacter::JointType >
{
	static const char* name(){ return "AnimCharacter_JointType"; }
	static const AnimCharacter_JointType_MPD* inst(){ static const AnimCharacter_JointType_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 0; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( AnimCharacter::JointType const*, int );
	static bool setprop( AnimCharacter::JointType*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( AnimCharacter::JointType const*, const mpd_Variant& );
	static bool setindex( AnimCharacter::JointType*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 3; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(AnimCharacter::JointType) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter_JointType, AnimCharacter::JointType);
template<> struct mpd_MetaType<AnimCharacter::JointType > : AnimCharacter_JointType_MPD {};
template<> struct mpd_MetaType<AnimCharacter::JointType const> : AnimCharacter_JointType_MPD {};
template<> struct mpd_MetaType<AnimCharacter_JointType_MPD> : AnimCharacter_JointType_MPD {};

struct AnimCharacter_TransformType_MPD : struct_MPD<AnimCharacter_TransformType_MPD, AnimCharacter::TransformType >
{
	static const char* name(){ return "AnimCharacter_TransformType"; }
	static const AnimCharacter_TransformType_MPD* inst(){ static const AnimCharacter_TransformType_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 0; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( AnimCharacter::TransformType const*, int );
	static bool setprop( AnimCharacter::TransformType*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( AnimCharacter::TransformType const*, const mpd_Variant& );
	static bool setindex( AnimCharacter::TransformType*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 4; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(AnimCharacter::TransformType) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter_TransformType, AnimCharacter::TransformType);
template<> struct mpd_MetaType<AnimCharacter::TransformType > : AnimCharacter_TransformType_MPD {};
template<> struct mpd_MetaType<AnimCharacter::TransformType const> : AnimCharacter_TransformType_MPD {};
template<> struct mpd_MetaType<AnimCharacter_TransformType_MPD> : AnimCharacter_TransformType_MPD {};

struct AnimCharacter_HitBox_MPD : struct_MPD<AnimCharacter_HitBox_MPD, AnimCharacter::HitBox >
{
	enum PIDS
	{
		PID_rotationAngles,
		PID_position,
		PID_extents,
		PID_multiplier,
	};

	static const char* name(){ return "AnimCharacter_HitBox"; }
	static const AnimCharacter_HitBox_MPD* inst(){ static const AnimCharacter_HitBox_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 4; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( AnimCharacter::HitBox const*, int );
	static bool setprop( AnimCharacter::HitBox*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( AnimCharacter::HitBox const*, const mpd_Variant& );
	static bool setindex( AnimCharacter::HitBox*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(AnimCharacter::HitBox) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter_HitBox, AnimCharacter::HitBox);
template<> struct mpd_MetaType<AnimCharacter::HitBox > : AnimCharacter_HitBox_MPD {};
template<> struct mpd_MetaType<AnimCharacter::HitBox const> : AnimCharacter_HitBox_MPD {};
template<> struct mpd_MetaType<AnimCharacter_HitBox_MPD> : AnimCharacter_HitBox_MPD {};

struct AnimCharacter_Body_MPD : struct_MPD<AnimCharacter_Body_MPD, AnimCharacter::Body >
{
	enum PIDS
	{
		PID_rotationAngles,
		PID_position,
		PID_type,
		PID_size,
	};

	static const char* name(){ return "AnimCharacter_Body"; }
	static const AnimCharacter_Body_MPD* inst(){ static const AnimCharacter_Body_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 4; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( AnimCharacter::Body const*, int );
	static bool setprop( AnimCharacter::Body*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( AnimCharacter::Body const*, const mpd_Variant& );
	static bool setindex( AnimCharacter::Body*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(AnimCharacter::Body) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter_Body, AnimCharacter::Body);
template<> struct mpd_MetaType<AnimCharacter::Body > : AnimCharacter_Body_MPD {};
template<> struct mpd_MetaType<AnimCharacter::Body const> : AnimCharacter_Body_MPD {};
template<> struct mpd_MetaType<AnimCharacter_Body_MPD> : AnimCharacter_Body_MPD {};

struct AnimCharacter_Joint_MPD : struct_MPD<AnimCharacter_Joint_MPD, AnimCharacter::Joint >
{
	enum PIDS
	{
		PID_parent_name,
		PID_type,
		PID_self_position,
		PID_self_rotationAngles,
		PID_prnt_position,
		PID_prnt_rotationAngles,
		PID_turn_limit_1,
		PID_turn_limit_2,
		PID_twist_limit,
	};

	static const char* name(){ return "AnimCharacter_Joint"; }
	static const AnimCharacter_Joint_MPD* inst(){ static const AnimCharacter_Joint_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 9; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( AnimCharacter::Joint const*, int );
	static bool setprop( AnimCharacter::Joint*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( AnimCharacter::Joint const*, const mpd_Variant& );
	static bool setindex( AnimCharacter::Joint*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(AnimCharacter::Joint) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter_Joint, AnimCharacter::Joint);
template<> struct mpd_MetaType<AnimCharacter::Joint > : AnimCharacter_Joint_MPD {};
template<> struct mpd_MetaType<AnimCharacter::Joint const> : AnimCharacter_Joint_MPD {};
template<> struct mpd_MetaType<AnimCharacter_Joint_MPD> : AnimCharacter_Joint_MPD {};

struct AnimCharacter_BoneInfo_MPD : struct_MPD<AnimCharacter_BoneInfo_MPD, AnimCharacter::BoneInfo >
{
	enum PIDS
	{
		PID_name,
		PID_hitbox,
		PID_body,
		PID_joint,
	};

	static const char* name(){ return "AnimCharacter_BoneInfo"; }
	static const AnimCharacter_BoneInfo_MPD* inst(){ static const AnimCharacter_BoneInfo_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 4; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( AnimCharacter::BoneInfo const*, int );
	static bool setprop( AnimCharacter::BoneInfo*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( AnimCharacter::BoneInfo const*, const mpd_Variant& );
	static bool setindex( AnimCharacter::BoneInfo*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(AnimCharacter::BoneInfo) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter_BoneInfo, AnimCharacter::BoneInfo);
template<> struct mpd_MetaType<AnimCharacter::BoneInfo > : AnimCharacter_BoneInfo_MPD {};
template<> struct mpd_MetaType<AnimCharacter::BoneInfo const> : AnimCharacter_BoneInfo_MPD {};
template<> struct mpd_MetaType<AnimCharacter_BoneInfo_MPD> : AnimCharacter_BoneInfo_MPD {};

struct AnimCharacter_Attachment_MPD : struct_MPD<AnimCharacter_Attachment_MPD, AnimCharacter::Attachment >
{
	enum PIDS
	{
		PID_name,
		PID_bone,
		PID_rotationAngles,
		PID_position,
	};

	static const char* name(){ return "AnimCharacter_Attachment"; }
	static const AnimCharacter_Attachment_MPD* inst(){ static const AnimCharacter_Attachment_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 4; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( AnimCharacter::Attachment const*, int );
	static bool setprop( AnimCharacter::Attachment*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( AnimCharacter::Attachment const*, const mpd_Variant& );
	static bool setindex( AnimCharacter::Attachment*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(AnimCharacter::Attachment) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter_Attachment, AnimCharacter::Attachment);
template<> struct mpd_MetaType<AnimCharacter::Attachment > : AnimCharacter_Attachment_MPD {};
template<> struct mpd_MetaType<AnimCharacter::Attachment const> : AnimCharacter_Attachment_MPD {};
template<> struct mpd_MetaType<AnimCharacter_Attachment_MPD> : AnimCharacter_Attachment_MPD {};

struct AnimCharacter_LayerTransform_MPD : struct_MPD<AnimCharacter_LayerTransform_MPD, AnimCharacter::LayerTransform >
{
	enum PIDS
	{
		PID_bone,
		PID_type,
		PID_posaxis,
		PID_angle,
		PID_base,
	};

	static const char* name(){ return "AnimCharacter_LayerTransform"; }
	static const AnimCharacter_LayerTransform_MPD* inst(){ static const AnimCharacter_LayerTransform_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 5; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( AnimCharacter::LayerTransform const*, int );
	static bool setprop( AnimCharacter::LayerTransform*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( AnimCharacter::LayerTransform const*, const mpd_Variant& );
	static bool setindex( AnimCharacter::LayerTransform*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(AnimCharacter::LayerTransform) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter_LayerTransform, AnimCharacter::LayerTransform);
template<> struct mpd_MetaType<AnimCharacter::LayerTransform > : AnimCharacter_LayerTransform_MPD {};
template<> struct mpd_MetaType<AnimCharacter::LayerTransform const> : AnimCharacter_LayerTransform_MPD {};
template<> struct mpd_MetaType<AnimCharacter_LayerTransform_MPD> : AnimCharacter_LayerTransform_MPD {};

struct AnimCharacter_LayerTransformArray_MPD : struct_MPD<AnimCharacter_LayerTransformArray_MPD, Array<AnimCharacter::LayerTransform> >
{
	static const char* name(){ return "AnimCharacter_LayerTransformArray"; }
	static const AnimCharacter_LayerTransformArray_MPD* inst(){ static const AnimCharacter_LayerTransformArray_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 0; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( Array<AnimCharacter::LayerTransform> const*, int );
	static bool setprop( Array<AnimCharacter::LayerTransform>*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ static const mpd_TypeInfo types[] = { { "int32_t", mpdt_Int32, 0 }, { "AnimCharacter_LayerTransform", mpdt_Struct, AnimCharacter_LayerTransform_MPD::inst() } }; return types; }
	static mpd_Variant getindex( Array<AnimCharacter::LayerTransform> const*, const mpd_Variant& );
	static bool setindex( Array<AnimCharacter::LayerTransform>*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(Array<AnimCharacter::LayerTransform>) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter_LayerTransformArray, Array<AnimCharacter::LayerTransform>);
template<> struct mpd_MetaType<Array<AnimCharacter::LayerTransform> > : AnimCharacter_LayerTransformArray_MPD {};
template<> struct mpd_MetaType<Array<AnimCharacter::LayerTransform> const> : AnimCharacter_LayerTransformArray_MPD {};
template<> struct mpd_MetaType<AnimCharacter_LayerTransformArray_MPD> : AnimCharacter_LayerTransformArray_MPD {};

struct AnimCharacter_Layer_MPD : struct_MPD<AnimCharacter_Layer_MPD, AnimCharacter::Layer >
{
	enum PIDS
	{
		PID_name,
		PID_transforms,
		PID_amount,
	};

	static const char* name(){ return "AnimCharacter_Layer"; }
	static const AnimCharacter_Layer_MPD* inst(){ static const AnimCharacter_Layer_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 3; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( AnimCharacter::Layer const*, int );
	static bool setprop( AnimCharacter::Layer*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( AnimCharacter::Layer const*, const mpd_Variant& );
	static bool setindex( AnimCharacter::Layer*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(AnimCharacter::Layer) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter_Layer, AnimCharacter::Layer);
template<> struct mpd_MetaType<AnimCharacter::Layer > : AnimCharacter_Layer_MPD {};
template<> struct mpd_MetaType<AnimCharacter::Layer const> : AnimCharacter_Layer_MPD {};
template<> struct mpd_MetaType<AnimCharacter_Layer_MPD> : AnimCharacter_Layer_MPD {};

struct AnimCharacter_MaskCmd_MPD : struct_MPD<AnimCharacter_MaskCmd_MPD, AnimCharacter::MaskCmd >
{
	enum PIDS
	{
		PID_bone,
		PID_weight,
		PID_children,
	};

	static const char* name(){ return "AnimCharacter_MaskCmd"; }
	static const AnimCharacter_MaskCmd_MPD* inst(){ static const AnimCharacter_MaskCmd_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 3; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( AnimCharacter::MaskCmd const*, int );
	static bool setprop( AnimCharacter::MaskCmd*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( AnimCharacter::MaskCmd const*, const mpd_Variant& );
	static bool setindex( AnimCharacter::MaskCmd*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(AnimCharacter::MaskCmd) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter_MaskCmd, AnimCharacter::MaskCmd);
template<> struct mpd_MetaType<AnimCharacter::MaskCmd > : AnimCharacter_MaskCmd_MPD {};
template<> struct mpd_MetaType<AnimCharacter::MaskCmd const> : AnimCharacter_MaskCmd_MPD {};
template<> struct mpd_MetaType<AnimCharacter_MaskCmd_MPD> : AnimCharacter_MaskCmd_MPD {};

struct AnimCharacter_MaskCmdArray_MPD : struct_MPD<AnimCharacter_MaskCmdArray_MPD, Array<AnimCharacter::MaskCmd> >
{
	enum PIDS
	{
		PID___size,
	};

	static const char* name(){ return "AnimCharacter_MaskCmdArray"; }
	static const AnimCharacter_MaskCmdArray_MPD* inst(){ static const AnimCharacter_MaskCmdArray_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 1; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( Array<AnimCharacter::MaskCmd> const*, int );
	static bool setprop( Array<AnimCharacter::MaskCmd>*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ static const mpd_TypeInfo types[] = { { "int32_t", mpdt_Int32, 0 }, { "AnimCharacter_MaskCmd", mpdt_Struct, AnimCharacter_MaskCmd_MPD::inst() } }; return types; }
	static mpd_Variant getindex( Array<AnimCharacter::MaskCmd> const*, const mpd_Variant& );
	static bool setindex( Array<AnimCharacter::MaskCmd>*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(Array<AnimCharacter::MaskCmd>) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter_MaskCmdArray, Array<AnimCharacter::MaskCmd>);
template<> struct mpd_MetaType<Array<AnimCharacter::MaskCmd> > : AnimCharacter_MaskCmdArray_MPD {};
template<> struct mpd_MetaType<Array<AnimCharacter::MaskCmd> const> : AnimCharacter_MaskCmdArray_MPD {};
template<> struct mpd_MetaType<AnimCharacter_MaskCmdArray_MPD> : AnimCharacter_MaskCmdArray_MPD {};

struct AnimCharacter_Mask_MPD : struct_MPD<AnimCharacter_Mask_MPD, AnimCharacter::Mask >
{
	enum PIDS
	{
		PID_name,
		PID_cmds,
	};

	static const char* name(){ return "AnimCharacter_Mask"; }
	static const AnimCharacter_Mask_MPD* inst(){ static const AnimCharacter_Mask_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 2; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( AnimCharacter::Mask const*, int );
	static bool setprop( AnimCharacter::Mask*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( AnimCharacter::Mask const*, const mpd_Variant& );
	static bool setindex( AnimCharacter::Mask*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(AnimCharacter::Mask) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter_Mask, AnimCharacter::Mask);
template<> struct mpd_MetaType<AnimCharacter::Mask > : AnimCharacter_Mask_MPD {};
template<> struct mpd_MetaType<AnimCharacter::Mask const> : AnimCharacter_Mask_MPD {};
template<> struct mpd_MetaType<AnimCharacter_Mask_MPD> : AnimCharacter_Mask_MPD {};

struct AnimCharacter_MaskArray_MPD : struct_MPD<AnimCharacter_MaskArray_MPD, Array<AnimCharacter::Mask> >
{
	enum PIDS
	{
		PID___size,
	};

	static const char* name(){ return "AnimCharacter_MaskArray"; }
	static const AnimCharacter_MaskArray_MPD* inst(){ static const AnimCharacter_MaskArray_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 1; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( Array<AnimCharacter::Mask> const*, int );
	static bool setprop( Array<AnimCharacter::Mask>*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ static const mpd_TypeInfo types[] = { { "int32_t", mpdt_Int32, 0 }, { "AnimCharacter_Mask", mpdt_Struct, AnimCharacter_Mask_MPD::inst() } }; return types; }
	static mpd_Variant getindex( Array<AnimCharacter::Mask> const*, const mpd_Variant& );
	static bool setindex( Array<AnimCharacter::Mask>*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(Array<AnimCharacter::Mask>) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter_MaskArray, Array<AnimCharacter::Mask>);
template<> struct mpd_MetaType<Array<AnimCharacter::Mask> > : AnimCharacter_MaskArray_MPD {};
template<> struct mpd_MetaType<Array<AnimCharacter::Mask> const> : AnimCharacter_MaskArray_MPD {};
template<> struct mpd_MetaType<AnimCharacter_MaskArray_MPD> : AnimCharacter_MaskArray_MPD {};

struct AnimCharacter_MPD : struct_MPD<AnimCharacter_MPD, AnimCharacter >
{
	enum PIDS
	{
		PID_mesh,
		PID_masks,
	};

	static const char* name(){ return "AnimCharacter"; }
	static const AnimCharacter_MPD* inst(){ static const AnimCharacter_MPD mpd; return &mpd; }
	static const mpd_KeyValue* metadata();

	static int propcount(){ return 2; }
	static const mpd_PropInfo* props();
	static mpd_Variant getprop( AnimCharacter const*, int );
	static bool setprop( AnimCharacter*, int, const mpd_Variant& );

	static const mpd_TypeInfo* indextypes(){ return 0; }
	static mpd_Variant getindex( AnimCharacter const*, const mpd_Variant& );
	static bool setindex( AnimCharacter*, const mpd_Variant&, const mpd_Variant& );

	static int valuecount(){ return 0; }
	static const mpd_EnumValue* values();

	static void dump( MPD_STATICDUMP_ARGS(AnimCharacter) );
};

MPD_DUMPDATA_WRAPPER(AnimCharacter, AnimCharacter);
template<> struct mpd_MetaType<AnimCharacter > : AnimCharacter_MPD {};
template<> struct mpd_MetaType<AnimCharacter const> : AnimCharacter_MPD {};
template<> struct mpd_MetaType<AnimCharacter_MPD> : AnimCharacter_MPD {};


#ifdef MPD_IMPL
const mpd_KeyValue* Vec2_MPD::metadata(){ static const mpd_KeyValue none = { 0, 0, 0, 0, 0, 0 }; return &none; }
const mpd_PropInfo* Vec2_MPD::props()
{
	static const mpd_KeyValue x_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue y_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_PropInfo data[] =
	{
		{ "x", 1, { "float", mpdt_Float32, 0 }, x_metadata },
		{ "y", 1, { "float", mpdt_Float32, 0 }, y_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant Vec2_MPD::getprop( Vec2 const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (float const&) obj->x;
	case 1: return (float const&) obj->y;
	default: return mpd_Variant();
	}
}
bool Vec2_MPD::setprop( Vec2* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->x = mpd_var_get<float >(val); return true;
	case 1: obj->y = mpd_var_get<float >(val); return true;
	default: return false;
	}
}
mpd_Variant Vec2_MPD::getindex( Vec2 const*, const mpd_Variant& ){ return mpd_Variant(); }
bool Vec2_MPD::setindex( Vec2*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* Vec2_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void Vec2_MPD::dump( MPD_STATICDUMP_ARGS(Vec2) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct Vec2\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( float, x, pdata->x );
		MPD_DUMP_PROP( float, y, pdata->y );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* Vec3_MPD::metadata(){ static const mpd_KeyValue none = { 0, 0, 0, 0, 0, 0 }; return &none; }
const mpd_PropInfo* Vec3_MPD::props()
{
	static const mpd_KeyValue x_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue y_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue z_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_PropInfo data[] =
	{
		{ "x", 1, { "float", mpdt_Float32, 0 }, x_metadata },
		{ "y", 1, { "float", mpdt_Float32, 0 }, y_metadata },
		{ "z", 1, { "float", mpdt_Float32, 0 }, z_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant Vec3_MPD::getprop( Vec3 const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (float const&) obj->x;
	case 1: return (float const&) obj->y;
	case 2: return (float const&) obj->z;
	default: return mpd_Variant();
	}
}
bool Vec3_MPD::setprop( Vec3* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->x = mpd_var_get<float >(val); return true;
	case 1: obj->y = mpd_var_get<float >(val); return true;
	case 2: obj->z = mpd_var_get<float >(val); return true;
	default: return false;
	}
}
mpd_Variant Vec3_MPD::getindex( Vec3 const*, const mpd_Variant& ){ return mpd_Variant(); }
bool Vec3_MPD::setindex( Vec3*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* Vec3_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void Vec3_MPD::dump( MPD_STATICDUMP_ARGS(Vec3) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct Vec3\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( float, x, pdata->x );
		MPD_DUMP_PROP( float, y, pdata->y );
		MPD_DUMP_PROP( float, z, pdata->z );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* String_MPD::metadata(){ static const mpd_KeyValue none = { 0, 0, 0, 0, 0, 0 }; return &none; }
const mpd_PropInfo* String_MPD::props()
{
	static const mpd_KeyValue data_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue size_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_PropInfo data[] =
	{
		{ "data", 4, { "mpd_StringView", mpdt_ConstString, 0 }, data_metadata },
		{ "size", 4, { "int32_t", mpdt_Int32, 0 }, size_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant String_MPD::getprop( String const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (mpd_StringView const&) mpd_StringView::create(obj->data(), obj->size());
	case 1: return (int32_t const&) obj->size();
	default: return mpd_Variant();
	}
}
bool String_MPD::setprop( String* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->assign(val.get_stringview().str, val.get_stringview().size); return true;
	case 1: obj->resize(val.get_int32()); return true;
	default: return false;
	}
}
mpd_Variant String_MPD::getindex( String const* obj, const mpd_Variant& key )
{
	return (int8_t const&)(*(obj))[mpd_var_get<int32_t >(key)];
}
bool String_MPD::setindex( String* obj, const mpd_Variant& key, const mpd_Variant& val )
{
	(*(obj))[mpd_var_get<int32_t >(key)] = mpd_var_get<int8_t >(val);
	return true;
}
const mpd_EnumValue* String_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void String_MPD::dump( MPD_STATICDUMP_ARGS(String) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct String\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( mpd_StringView, data, mpd_StringView::create(pdata->data(), pdata->size()) );
		MPD_DUMP_PROP( int32_t, size, pdata->size() );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* Vec3Array_MPD::metadata(){ static const mpd_KeyValue none = { 0, 0, 0, 0, 0, 0 }; return &none; }
const mpd_PropInfo* Vec3Array_MPD::props()
{
	static const mpd_KeyValue size_metadata[] =
	{
		{ "label", 5, "Size", 4, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "size", 4, { "int32_t", mpdt_Int32, 0 }, size_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant Vec3Array_MPD::getprop( Array<Vec3> const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (int32_t const&) obj->size();
	default: return mpd_Variant();
	}
}
bool Vec3Array_MPD::setprop( Array<Vec3>* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->resize(val.get_int32()); return true;
	default: return false;
	}
}
mpd_Variant Vec3Array_MPD::getindex( Array<Vec3> const* obj, const mpd_Variant& key )
{
	return (Vec3 const&)(*(obj))[mpd_var_get<int32_t >(key)];
}
bool Vec3Array_MPD::setindex( Array<Vec3>* obj, const mpd_Variant& key, const mpd_Variant& val )
{
	(*(obj))[mpd_var_get<int32_t >(key)] = mpd_var_get<Vec3 >(val);
	return true;
}
const mpd_EnumValue* Vec3Array_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void Vec3Array_MPD::dump( MPD_STATICDUMP_ARGS(Array<Vec3>) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct Vec3Array\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( int32_t, size, pdata->size() );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* AnimCharacter_BodyType_MPD::metadata()
{
	static const mpd_KeyValue data[] =
	{
		{ "label", 5, "Body type", 9, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return data;
}
const mpd_PropInfo* AnimCharacter_BodyType_MPD::props(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_PropInfo none = { 0, 0, { 0, mpdt_None, 0 }, &kvnone }; return &none; }
mpd_Variant AnimCharacter_BodyType_MPD::getprop( AnimCharacter::BodyType const*, int ){ return mpd_Variant(); }
bool AnimCharacter_BodyType_MPD::setprop( AnimCharacter::BodyType*, int, const mpd_Variant& ){ return false; }
mpd_Variant AnimCharacter_BodyType_MPD::getindex( AnimCharacter::BodyType const*, const mpd_Variant& ){ return mpd_Variant(); }
bool AnimCharacter_BodyType_MPD::setindex( AnimCharacter::BodyType*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* AnimCharacter_BodyType_MPD::values()
{
	static const mpd_KeyValue BodyType_None_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue BodyType_Sphere_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue BodyType_Capsule_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue BodyType_Box_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_EnumValue data[] =
	{
		{ "BodyType_None", 13, AnimCharacter::BodyType_None, BodyType_None_metadata },
		{ "BodyType_Sphere", 15, AnimCharacter::BodyType_Sphere, BodyType_Sphere_metadata },
		{ "BodyType_Capsule", 16, AnimCharacter::BodyType_Capsule, BodyType_Capsule_metadata },
		{ "BodyType_Box", 12, AnimCharacter::BodyType_Box, BodyType_Box_metadata },
		{ 0, 0, 0, 0 },
	};
	return data;
}
void AnimCharacter_BodyType_MPD::dump( MPD_STATICDUMP_ARGS(AnimCharacter::BodyType) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "AnimCharacter_BodyType::%s (%d)", value2name( *pdata ), (int)*pdata );
}

const mpd_KeyValue* AnimCharacter_JointType_MPD::metadata()
{
	static const mpd_KeyValue data[] =
	{
		{ "label", 5, "Joint type", 10, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return data;
}
const mpd_PropInfo* AnimCharacter_JointType_MPD::props(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_PropInfo none = { 0, 0, { 0, mpdt_None, 0 }, &kvnone }; return &none; }
mpd_Variant AnimCharacter_JointType_MPD::getprop( AnimCharacter::JointType const*, int ){ return mpd_Variant(); }
bool AnimCharacter_JointType_MPD::setprop( AnimCharacter::JointType*, int, const mpd_Variant& ){ return false; }
mpd_Variant AnimCharacter_JointType_MPD::getindex( AnimCharacter::JointType const*, const mpd_Variant& ){ return mpd_Variant(); }
bool AnimCharacter_JointType_MPD::setindex( AnimCharacter::JointType*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* AnimCharacter_JointType_MPD::values()
{
	static const mpd_KeyValue JointType_None_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue JointType_Hinge_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue JointType_ConeTwist_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_EnumValue data[] =
	{
		{ "JointType_None", 14, AnimCharacter::JointType_None, JointType_None_metadata },
		{ "JointType_Hinge", 15, AnimCharacter::JointType_Hinge, JointType_Hinge_metadata },
		{ "JointType_ConeTwist", 19, AnimCharacter::JointType_ConeTwist, JointType_ConeTwist_metadata },
		{ 0, 0, 0, 0 },
	};
	return data;
}
void AnimCharacter_JointType_MPD::dump( MPD_STATICDUMP_ARGS(AnimCharacter::JointType) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "AnimCharacter_JointType::%s (%d)", value2name( *pdata ), (int)*pdata );
}

const mpd_KeyValue* AnimCharacter_TransformType_MPD::metadata()
{
	static const mpd_KeyValue data[] =
	{
		{ "label", 5, "Transform type", 14, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return data;
}
const mpd_PropInfo* AnimCharacter_TransformType_MPD::props(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_PropInfo none = { 0, 0, { 0, mpdt_None, 0 }, &kvnone }; return &none; }
mpd_Variant AnimCharacter_TransformType_MPD::getprop( AnimCharacter::TransformType const*, int ){ return mpd_Variant(); }
bool AnimCharacter_TransformType_MPD::setprop( AnimCharacter::TransformType*, int, const mpd_Variant& ){ return false; }
mpd_Variant AnimCharacter_TransformType_MPD::getindex( AnimCharacter::TransformType const*, const mpd_Variant& ){ return mpd_Variant(); }
bool AnimCharacter_TransformType_MPD::setindex( AnimCharacter::TransformType*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* AnimCharacter_TransformType_MPD::values()
{
	static const mpd_KeyValue TransformType_None_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue TransformType_UndoParent_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue TransformType_Move_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_KeyValue TransformType_Rotate_metadata[] = { { 0, 0, 0, 0, 0, 0 } };
	static const mpd_EnumValue data[] =
	{
		{ "TransformType_None", 18, AnimCharacter::TransformType_None, TransformType_None_metadata },
		{ "TransformType_UndoParent", 24, AnimCharacter::TransformType_UndoParent, TransformType_UndoParent_metadata },
		{ "TransformType_Move", 18, AnimCharacter::TransformType_Move, TransformType_Move_metadata },
		{ "TransformType_Rotate", 20, AnimCharacter::TransformType_Rotate, TransformType_Rotate_metadata },
		{ 0, 0, 0, 0 },
	};
	return data;
}
void AnimCharacter_TransformType_MPD::dump( MPD_STATICDUMP_ARGS(AnimCharacter::TransformType) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "AnimCharacter_TransformType::%s (%d)", value2name( *pdata ), (int)*pdata );
}

const mpd_KeyValue* AnimCharacter_HitBox_MPD::metadata()
{
	static const mpd_KeyValue data[] =
	{
		{ "label", 5, "Hitbox", 6, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return data;
}
const mpd_PropInfo* AnimCharacter_HitBox_MPD::props()
{
	static const mpd_KeyValue rotationAngles_metadata[] =
	{
		{ "label", 5, "Rotation", 8, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue position_metadata[] =
	{
		{ "label", 5, "Position", 8, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue extents_metadata[] =
	{
		{ "label", 5, "Extents", 7, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue multiplier_metadata[] =
	{
		{ "label", 5, "Multiplier", 10, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "rotationAngles", 14, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, rotationAngles_metadata },
		{ "position", 8, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, position_metadata },
		{ "extents", 7, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, extents_metadata },
		{ "multiplier", 10, { "float", mpdt_Float32, 0 }, multiplier_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant AnimCharacter_HitBox_MPD::getprop( AnimCharacter::HitBox const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (Vec3 const&) Q2EA(obj->rotation);
	case 1: return (Vec3 const&) obj->position;
	case 2: return (Vec3 const&) obj->extents;
	case 3: return (float const&) obj->multiplier;
	default: return mpd_Variant();
	}
}
bool AnimCharacter_HitBox_MPD::setprop( AnimCharacter::HitBox* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->rotation = EA2Q(val.get_obj<Vec3>()); return true;
	case 1: obj->position = mpd_var_get<Vec3 >(val); return true;
	case 2: obj->extents = mpd_var_get<Vec3 >(val); return true;
	case 3: obj->multiplier = mpd_var_get<float >(val); return true;
	default: return false;
	}
}
mpd_Variant AnimCharacter_HitBox_MPD::getindex( AnimCharacter::HitBox const*, const mpd_Variant& ){ return mpd_Variant(); }
bool AnimCharacter_HitBox_MPD::setindex( AnimCharacter::HitBox*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* AnimCharacter_HitBox_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void AnimCharacter_HitBox_MPD::dump( MPD_STATICDUMP_ARGS(AnimCharacter::HitBox) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct AnimCharacter_HitBox\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( Vec3, rotationAngles, Q2EA(pdata->rotation) );
		MPD_DUMP_PROP( Vec3, position, pdata->position );
		MPD_DUMP_PROP( Vec3, extents, pdata->extents );
		MPD_DUMP_PROP( float, multiplier, pdata->multiplier );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* AnimCharacter_Body_MPD::metadata()
{
	static const mpd_KeyValue data[] =
	{
		{ "label", 5, "Body", 4, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return data;
}
const mpd_PropInfo* AnimCharacter_Body_MPD::props()
{
	static const mpd_KeyValue rotationAngles_metadata[] =
	{
		{ "label", 5, "Rotation", 8, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue position_metadata[] =
	{
		{ "label", 5, "Position", 8, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue type_metadata[] =
	{
		{ "label", 5, "Body type", 9, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue size_metadata[] =
	{
		{ "label", 5, "Size", 4, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "rotationAngles", 14, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, rotationAngles_metadata },
		{ "position", 8, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, position_metadata },
		{ "type", 4, { "AnimCharacter_BodyType", mpdt_Enum, AnimCharacter_BodyType_MPD::inst() }, type_metadata },
		{ "size", 4, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, size_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant AnimCharacter_Body_MPD::getprop( AnimCharacter::Body const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (Vec3 const&) Q2EA(obj->rotation);
	case 1: return (Vec3 const&) obj->position;
	case 2: return mpd_Variant( (AnimCharacter::BodyType const&) obj->type, mpd_Variant::Enum );
	case 3: return (Vec3 const&) obj->size;
	default: return mpd_Variant();
	}
}
bool AnimCharacter_Body_MPD::setprop( AnimCharacter::Body* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->rotation = EA2Q(val.get_obj<Vec3>()); return true;
	case 1: obj->position = mpd_var_get<Vec3 >(val); return true;
	case 2: obj->type = mpd_var_get<AnimCharacter::BodyType >(val); return true;
	case 3: obj->size = mpd_var_get<Vec3 >(val); return true;
	default: return false;
	}
}
mpd_Variant AnimCharacter_Body_MPD::getindex( AnimCharacter::Body const*, const mpd_Variant& ){ return mpd_Variant(); }
bool AnimCharacter_Body_MPD::setindex( AnimCharacter::Body*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* AnimCharacter_Body_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void AnimCharacter_Body_MPD::dump( MPD_STATICDUMP_ARGS(AnimCharacter::Body) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct AnimCharacter_Body\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( Vec3, rotationAngles, Q2EA(pdata->rotation) );
		MPD_DUMP_PROP( Vec3, position, pdata->position );
		MPD_DUMP_PROP( AnimCharacter::BodyType, type, pdata->type );
		MPD_DUMP_PROP( Vec3, size, pdata->size );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* AnimCharacter_Joint_MPD::metadata()
{
	static const mpd_KeyValue data[] =
	{
		{ "label", 5, "Joint", 5, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return data;
}
const mpd_PropInfo* AnimCharacter_Joint_MPD::props()
{
	static const mpd_KeyValue parent_name_metadata[] =
	{
		{ "label", 5, "Parent name", 11, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue type_metadata[] =
	{
		{ "label", 5, "Joint type", 10, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue self_position_metadata[] =
	{
		{ "label", 5, "Local position", 14, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue self_rotationAngles_metadata[] =
	{
		{ "label", 5, "Local rotation", 14, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue prnt_position_metadata[] =
	{
		{ "label", 5, "Parent position", 15, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue prnt_rotationAngles_metadata[] =
	{
		{ "label", 5, "Parent rotation", 15, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue turn_limit_1_metadata[] =
	{
		{ "label", 5, "Turn limit 1 (Min/X)", 20, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue turn_limit_2_metadata[] =
	{
		{ "label", 5, "Turn limit 2 (Max/Y)", 20, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue twist_limit_metadata[] =
	{
		{ "label", 5, "Twist limit 2 (Z)", 17, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "parent_name", 11, { "String", mpdt_Struct, String_MPD::inst() }, parent_name_metadata },
		{ "type", 4, { "AnimCharacter_JointType", mpdt_Enum, AnimCharacter_JointType_MPD::inst() }, type_metadata },
		{ "self_position", 13, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, self_position_metadata },
		{ "self_rotationAngles", 19, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, self_rotationAngles_metadata },
		{ "prnt_position", 13, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, prnt_position_metadata },
		{ "prnt_rotationAngles", 19, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, prnt_rotationAngles_metadata },
		{ "turn_limit_1", 12, { "float", mpdt_Float32, 0 }, turn_limit_1_metadata },
		{ "turn_limit_2", 12, { "float", mpdt_Float32, 0 }, turn_limit_2_metadata },
		{ "twist_limit", 11, { "float", mpdt_Float32, 0 }, twist_limit_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant AnimCharacter_Joint_MPD::getprop( AnimCharacter::Joint const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (String const&) obj->parent_name;
	case 1: return mpd_Variant( (AnimCharacter::JointType const&) obj->type, mpd_Variant::Enum );
	case 2: return (Vec3 const&) obj->self_position;
	case 3: return (Vec3 const&) Q2EA(obj->self_rotation);
	case 4: return (Vec3 const&) obj->prnt_position;
	case 5: return (Vec3 const&) Q2EA(obj->prnt_rotation);
	case 6: return (float const&) obj->turn_limit_1;
	case 7: return (float const&) obj->turn_limit_2;
	case 8: return (float const&) obj->twist_limit;
	default: return mpd_Variant();
	}
}
bool AnimCharacter_Joint_MPD::setprop( AnimCharacter::Joint* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->parent_name = mpd_var_get<String >(val); return true;
	case 1: obj->type = mpd_var_get<AnimCharacter::JointType >(val); return true;
	case 2: obj->self_position = mpd_var_get<Vec3 >(val); return true;
	case 3: obj->self_rotation = EA2Q(val.get_obj<Vec3>()); return true;
	case 4: obj->prnt_position = mpd_var_get<Vec3 >(val); return true;
	case 5: obj->prnt_rotation = EA2Q(val.get_obj<Vec3>()); return true;
	case 6: obj->turn_limit_1 = mpd_var_get<float >(val); return true;
	case 7: obj->turn_limit_2 = mpd_var_get<float >(val); return true;
	case 8: obj->twist_limit = mpd_var_get<float >(val); return true;
	default: return false;
	}
}
mpd_Variant AnimCharacter_Joint_MPD::getindex( AnimCharacter::Joint const*, const mpd_Variant& ){ return mpd_Variant(); }
bool AnimCharacter_Joint_MPD::setindex( AnimCharacter::Joint*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* AnimCharacter_Joint_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void AnimCharacter_Joint_MPD::dump( MPD_STATICDUMP_ARGS(AnimCharacter::Joint) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct AnimCharacter_Joint\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( String, parent_name, pdata->parent_name );
		MPD_DUMP_PROP( AnimCharacter::JointType, type, pdata->type );
		MPD_DUMP_PROP( Vec3, self_position, pdata->self_position );
		MPD_DUMP_PROP( Vec3, self_rotationAngles, Q2EA(pdata->self_rotation) );
		MPD_DUMP_PROP( Vec3, prnt_position, pdata->prnt_position );
		MPD_DUMP_PROP( Vec3, prnt_rotationAngles, Q2EA(pdata->prnt_rotation) );
		MPD_DUMP_PROP( float, turn_limit_1, pdata->turn_limit_1 );
		MPD_DUMP_PROP( float, turn_limit_2, pdata->turn_limit_2 );
		MPD_DUMP_PROP( float, twist_limit, pdata->twist_limit );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* AnimCharacter_BoneInfo_MPD::metadata()
{
	static const mpd_KeyValue data[] =
	{
		{ "label", 5, "Bone info", 9, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return data;
}
const mpd_PropInfo* AnimCharacter_BoneInfo_MPD::props()
{
	static const mpd_KeyValue name_metadata[] =
	{
		{ "label", 5, "Name", 4, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue hitbox_metadata[] =
	{
		{ "label", 5, "Hitbox", 6, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue body_metadata[] =
	{
		{ "label", 5, "Body", 4, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue joint_metadata[] =
	{
		{ "label", 5, "Joint", 5, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "name", 4, { "String", mpdt_Struct, String_MPD::inst() }, name_metadata },
		{ "hitbox", 6, { "AnimCharacter_HitBox", mpdt_Struct, AnimCharacter_HitBox_MPD::inst() }, hitbox_metadata },
		{ "body", 4, { "AnimCharacter_Body", mpdt_Struct, AnimCharacter_Body_MPD::inst() }, body_metadata },
		{ "joint", 5, { "AnimCharacter_Joint", mpdt_Struct, AnimCharacter_Joint_MPD::inst() }, joint_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant AnimCharacter_BoneInfo_MPD::getprop( AnimCharacter::BoneInfo const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (String const&) obj->name;
	case 1: return (AnimCharacter::HitBox const&) obj->hitbox;
	case 2: return (AnimCharacter::Body const&) obj->body;
	case 3: return (AnimCharacter::Joint const&) obj->joint;
	default: return mpd_Variant();
	}
}
bool AnimCharacter_BoneInfo_MPD::setprop( AnimCharacter::BoneInfo* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->name = mpd_var_get<String >(val); return true;
	case 1: obj->hitbox = mpd_var_get<AnimCharacter::HitBox >(val); return true;
	case 2: obj->body = mpd_var_get<AnimCharacter::Body >(val); return true;
	case 3: obj->joint = mpd_var_get<AnimCharacter::Joint >(val); return true;
	default: return false;
	}
}
mpd_Variant AnimCharacter_BoneInfo_MPD::getindex( AnimCharacter::BoneInfo const*, const mpd_Variant& ){ return mpd_Variant(); }
bool AnimCharacter_BoneInfo_MPD::setindex( AnimCharacter::BoneInfo*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* AnimCharacter_BoneInfo_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void AnimCharacter_BoneInfo_MPD::dump( MPD_STATICDUMP_ARGS(AnimCharacter::BoneInfo) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct AnimCharacter_BoneInfo\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( String, name, pdata->name );
		MPD_DUMP_PROP( AnimCharacter::HitBox, hitbox, pdata->hitbox );
		MPD_DUMP_PROP( AnimCharacter::Body, body, pdata->body );
		MPD_DUMP_PROP( AnimCharacter::Joint, joint, pdata->joint );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* AnimCharacter_Attachment_MPD::metadata()
{
	static const mpd_KeyValue data[] =
	{
		{ "label", 5, "Attachment", 10, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return data;
}
const mpd_PropInfo* AnimCharacter_Attachment_MPD::props()
{
	static const mpd_KeyValue name_metadata[] =
	{
		{ "label", 5, "Name", 4, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue bone_metadata[] =
	{
		{ "label", 5, "Bone", 4, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue rotationAngles_metadata[] =
	{
		{ "label", 5, "Rotation", 8, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue position_metadata[] =
	{
		{ "label", 5, "Position", 8, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "name", 4, { "String", mpdt_Struct, String_MPD::inst() }, name_metadata },
		{ "bone", 4, { "String", mpdt_Struct, String_MPD::inst() }, bone_metadata },
		{ "rotationAngles", 14, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, rotationAngles_metadata },
		{ "position", 8, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, position_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant AnimCharacter_Attachment_MPD::getprop( AnimCharacter::Attachment const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (String const&) obj->name;
	case 1: return (String const&) obj->bone;
	case 2: return (Vec3 const&) Q2EA(obj->rotation);
	case 3: return (Vec3 const&) obj->position;
	default: return mpd_Variant();
	}
}
bool AnimCharacter_Attachment_MPD::setprop( AnimCharacter::Attachment* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->name = mpd_var_get<String >(val); return true;
	case 1: obj->bone = mpd_var_get<String >(val); return true;
	case 2: obj->rotation = EA2Q(val.get_obj<Vec3>()); return true;
	case 3: obj->position = mpd_var_get<Vec3 >(val); return true;
	default: return false;
	}
}
mpd_Variant AnimCharacter_Attachment_MPD::getindex( AnimCharacter::Attachment const*, const mpd_Variant& ){ return mpd_Variant(); }
bool AnimCharacter_Attachment_MPD::setindex( AnimCharacter::Attachment*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* AnimCharacter_Attachment_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void AnimCharacter_Attachment_MPD::dump( MPD_STATICDUMP_ARGS(AnimCharacter::Attachment) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct AnimCharacter_Attachment\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( String, name, pdata->name );
		MPD_DUMP_PROP( String, bone, pdata->bone );
		MPD_DUMP_PROP( Vec3, rotationAngles, Q2EA(pdata->rotation) );
		MPD_DUMP_PROP( Vec3, position, pdata->position );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* AnimCharacter_LayerTransform_MPD::metadata()
{
	static const mpd_KeyValue data[] =
	{
		{ "label", 5, "Layer transform", 15, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return data;
}
const mpd_PropInfo* AnimCharacter_LayerTransform_MPD::props()
{
	static const mpd_KeyValue bone_metadata[] =
	{
		{ "label", 5, "Bone", 4, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue type_metadata[] =
	{
		{ "label", 5, "Transform type", 14, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue posaxis_metadata[] =
	{
		{ "label", 5, "Position/Axis", 13, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue angle_metadata[] =
	{
		{ "label", 5, "Rotation angle", 14, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue base_metadata[] =
	{
		{ "label", 5, "Base offset", 11, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "bone", 4, { "String", mpdt_Struct, String_MPD::inst() }, bone_metadata },
		{ "type", 4, { "AnimCharacter_TransformType", mpdt_Enum, AnimCharacter_TransformType_MPD::inst() }, type_metadata },
		{ "posaxis", 7, { "Vec3", mpdt_Struct, Vec3_MPD::inst() }, posaxis_metadata },
		{ "angle", 5, { "float", mpdt_Float32, 0 }, angle_metadata },
		{ "base", 4, { "float", mpdt_Float32, 0 }, base_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant AnimCharacter_LayerTransform_MPD::getprop( AnimCharacter::LayerTransform const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (String const&) obj->bone;
	case 1: return mpd_Variant( (AnimCharacter::TransformType const&) obj->type, mpd_Variant::Enum );
	case 2: return (Vec3 const&) obj->posaxis;
	case 3: return (float const&) obj->angle;
	case 4: return (float const&) obj->base;
	default: return mpd_Variant();
	}
}
bool AnimCharacter_LayerTransform_MPD::setprop( AnimCharacter::LayerTransform* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->bone = mpd_var_get<String >(val); return true;
	case 1: obj->type = mpd_var_get<AnimCharacter::TransformType >(val); return true;
	case 2: obj->posaxis = mpd_var_get<Vec3 >(val); return true;
	case 3: obj->angle = mpd_var_get<float >(val); return true;
	case 4: obj->base = mpd_var_get<float >(val); return true;
	default: return false;
	}
}
mpd_Variant AnimCharacter_LayerTransform_MPD::getindex( AnimCharacter::LayerTransform const*, const mpd_Variant& ){ return mpd_Variant(); }
bool AnimCharacter_LayerTransform_MPD::setindex( AnimCharacter::LayerTransform*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* AnimCharacter_LayerTransform_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void AnimCharacter_LayerTransform_MPD::dump( MPD_STATICDUMP_ARGS(AnimCharacter::LayerTransform) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct AnimCharacter_LayerTransform\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( String, bone, pdata->bone );
		MPD_DUMP_PROP( AnimCharacter::TransformType, type, pdata->type );
		MPD_DUMP_PROP( Vec3, posaxis, pdata->posaxis );
		MPD_DUMP_PROP( float, angle, pdata->angle );
		MPD_DUMP_PROP( float, base, pdata->base );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* AnimCharacter_LayerTransformArray_MPD::metadata(){ static const mpd_KeyValue none = { 0, 0, 0, 0, 0, 0 }; return &none; }
const mpd_PropInfo* AnimCharacter_LayerTransformArray_MPD::props(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_PropInfo none = { 0, 0, { 0, mpdt_None, 0 }, &kvnone }; return &none; }
mpd_Variant AnimCharacter_LayerTransformArray_MPD::getprop( Array<AnimCharacter::LayerTransform> const*, int ){ return mpd_Variant(); }
bool AnimCharacter_LayerTransformArray_MPD::setprop( Array<AnimCharacter::LayerTransform>*, int, const mpd_Variant& ){ return false; }
mpd_Variant AnimCharacter_LayerTransformArray_MPD::getindex( Array<AnimCharacter::LayerTransform> const* obj, const mpd_Variant& key )
{
	return (AnimCharacter::LayerTransform const&)(*(obj))[mpd_var_get<int32_t >(key)];
}
bool AnimCharacter_LayerTransformArray_MPD::setindex( Array<AnimCharacter::LayerTransform>* obj, const mpd_Variant& key, const mpd_Variant& val )
{
	(*(obj))[mpd_var_get<int32_t >(key)] = mpd_var_get<AnimCharacter::LayerTransform >(val);
	return true;
}
const mpd_EnumValue* AnimCharacter_LayerTransformArray_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void AnimCharacter_LayerTransformArray_MPD::dump( MPD_STATICDUMP_ARGS(Array<AnimCharacter::LayerTransform>) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct AnimCharacter_LayerTransformArray\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* AnimCharacter_Layer_MPD::metadata(){ static const mpd_KeyValue none = { 0, 0, 0, 0, 0, 0 }; return &none; }
const mpd_PropInfo* AnimCharacter_Layer_MPD::props()
{
	static const mpd_KeyValue name_metadata[] =
	{
		{ "label", 5, "Name", 4, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue transforms_metadata[] =
	{
		{ "label", 5, "Layer transforms", 16, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue amount_metadata[] =
	{
		{ "label", 5, "Test amount", 11, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "name", 4, { "String", mpdt_Struct, String_MPD::inst() }, name_metadata },
		{ "transforms", 10, { "AnimCharacter_LayerTransformArray", mpdt_Struct, AnimCharacter_LayerTransformArray_MPD::inst() }, transforms_metadata },
		{ "amount", 6, { "float", mpdt_Float32, 0 }, amount_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant AnimCharacter_Layer_MPD::getprop( AnimCharacter::Layer const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (String const&) obj->name;
	case 1: return (Array<AnimCharacter::LayerTransform> const&) obj->transforms;
	case 2: return (float const&) obj->amount;
	default: return mpd_Variant();
	}
}
bool AnimCharacter_Layer_MPD::setprop( AnimCharacter::Layer* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->name = mpd_var_get<String >(val); return true;
	case 1: obj->transforms = mpd_var_get<Array<AnimCharacter::LayerTransform> >(val); return true;
	case 2: obj->amount = mpd_var_get<float >(val); return true;
	default: return false;
	}
}
mpd_Variant AnimCharacter_Layer_MPD::getindex( AnimCharacter::Layer const*, const mpd_Variant& ){ return mpd_Variant(); }
bool AnimCharacter_Layer_MPD::setindex( AnimCharacter::Layer*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* AnimCharacter_Layer_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void AnimCharacter_Layer_MPD::dump( MPD_STATICDUMP_ARGS(AnimCharacter::Layer) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct AnimCharacter_Layer\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( String, name, pdata->name );
		MPD_DUMP_PROP( Array<AnimCharacter::LayerTransform>, transforms, pdata->transforms );
		MPD_DUMP_PROP( float, amount, pdata->amount );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* AnimCharacter_MaskCmd_MPD::metadata()
{
	static const mpd_KeyValue data[] =
	{
		{ "label", 5, "Mask command", 12, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return data;
}
const mpd_PropInfo* AnimCharacter_MaskCmd_MPD::props()
{
	static const mpd_KeyValue bone_metadata[] =
	{
		{ "label", 5, "Bone", 4, 0, (float) 0 },
		{ "edit", 4, "bone", 4, 0, (float) 0 },
		{ "edit_requestReload", 18, "true", 4, 1, (float) 1 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue weight_metadata[] =
	{
		{ "label", 5, "Weight", 6, 0, (float) 0 },
		{ "min", 3, "0", 1, 0, (float) 0 },
		{ "max", 3, "1", 1, 1, (float) 1 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue children_metadata[] =
	{
		{ "label", 5, "Include children", 16, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "bone", 4, { "String", mpdt_Struct, String_MPD::inst() }, bone_metadata },
		{ "weight", 6, { "float", mpdt_Float32, 0 }, weight_metadata },
		{ "children", 8, { "bool", mpdt_Bool, 0 }, children_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant AnimCharacter_MaskCmd_MPD::getprop( AnimCharacter::MaskCmd const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (String const&) obj->bone;
	case 1: return (float const&) obj->weight;
	case 2: return (bool const&) obj->children;
	default: return mpd_Variant();
	}
}
bool AnimCharacter_MaskCmd_MPD::setprop( AnimCharacter::MaskCmd* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->bone = mpd_var_get<String >(val); return true;
	case 1: obj->weight = mpd_var_get<float >(val); return true;
	case 2: obj->children = mpd_var_get<bool >(val); return true;
	default: return false;
	}
}
mpd_Variant AnimCharacter_MaskCmd_MPD::getindex( AnimCharacter::MaskCmd const*, const mpd_Variant& ){ return mpd_Variant(); }
bool AnimCharacter_MaskCmd_MPD::setindex( AnimCharacter::MaskCmd*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* AnimCharacter_MaskCmd_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void AnimCharacter_MaskCmd_MPD::dump( MPD_STATICDUMP_ARGS(AnimCharacter::MaskCmd) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct AnimCharacter_MaskCmd\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( String, bone, pdata->bone );
		MPD_DUMP_PROP( float, weight, pdata->weight );
		MPD_DUMP_PROP( bool, children, pdata->children );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* AnimCharacter_MaskCmdArray_MPD::metadata(){ static const mpd_KeyValue none = { 0, 0, 0, 0, 0, 0 }; return &none; }
const mpd_PropInfo* AnimCharacter_MaskCmdArray_MPD::props()
{
	static const mpd_KeyValue __size_metadata[] =
	{
		{ "visible", 7, "false", 5, 0, (float) 0 },
		{ "min", 3, "0", 1, 0, (float) 0 },
		{ "max", 3, "1000", 4, 1000, (float) 1000 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "__size", 6, { "int32_t", mpdt_Int32, 0 }, __size_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant AnimCharacter_MaskCmdArray_MPD::getprop( Array<AnimCharacter::MaskCmd> const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (int32_t const&) obj->size();
	default: return mpd_Variant();
	}
}
bool AnimCharacter_MaskCmdArray_MPD::setprop( Array<AnimCharacter::MaskCmd>* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->resize(val.get_int32()); return true;
	default: return false;
	}
}
mpd_Variant AnimCharacter_MaskCmdArray_MPD::getindex( Array<AnimCharacter::MaskCmd> const* obj, const mpd_Variant& key )
{
	return (AnimCharacter::MaskCmd const&)(*(obj))[mpd_var_get<int32_t >(key)];
}
bool AnimCharacter_MaskCmdArray_MPD::setindex( Array<AnimCharacter::MaskCmd>* obj, const mpd_Variant& key, const mpd_Variant& val )
{
	(*(obj))[mpd_var_get<int32_t >(key)] = mpd_var_get<AnimCharacter::MaskCmd >(val);
	return true;
}
const mpd_EnumValue* AnimCharacter_MaskCmdArray_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void AnimCharacter_MaskCmdArray_MPD::dump( MPD_STATICDUMP_ARGS(Array<AnimCharacter::MaskCmd>) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct AnimCharacter_MaskCmdArray\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( int32_t, __size, pdata->size() );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* AnimCharacter_Mask_MPD::metadata()
{
	static const mpd_KeyValue data[] =
	{
		{ "label", 5, "Mask", 4, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return data;
}
const mpd_PropInfo* AnimCharacter_Mask_MPD::props()
{
	static const mpd_KeyValue name_metadata[] =
	{
		{ "label", 5, "Name", 4, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue cmds_metadata[] =
	{
		{ "label", 5, "Mask commands", 13, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "name", 4, { "String", mpdt_Struct, String_MPD::inst() }, name_metadata },
		{ "cmds", 4, { "AnimCharacter_MaskCmdArray", mpdt_Struct, AnimCharacter_MaskCmdArray_MPD::inst() }, cmds_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant AnimCharacter_Mask_MPD::getprop( AnimCharacter::Mask const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (String const&) obj->name;
	case 1: return (Array<AnimCharacter::MaskCmd> const&) obj->cmds;
	default: return mpd_Variant();
	}
}
bool AnimCharacter_Mask_MPD::setprop( AnimCharacter::Mask* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->name = mpd_var_get<String >(val); return true;
	case 1: obj->cmds = mpd_var_get<Array<AnimCharacter::MaskCmd> >(val); return true;
	default: return false;
	}
}
mpd_Variant AnimCharacter_Mask_MPD::getindex( AnimCharacter::Mask const*, const mpd_Variant& ){ return mpd_Variant(); }
bool AnimCharacter_Mask_MPD::setindex( AnimCharacter::Mask*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* AnimCharacter_Mask_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void AnimCharacter_Mask_MPD::dump( MPD_STATICDUMP_ARGS(AnimCharacter::Mask) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct AnimCharacter_Mask\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( String, name, pdata->name );
		MPD_DUMP_PROP( Array<AnimCharacter::MaskCmd>, cmds, pdata->cmds );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* AnimCharacter_MaskArray_MPD::metadata(){ static const mpd_KeyValue none = { 0, 0, 0, 0, 0, 0 }; return &none; }
const mpd_PropInfo* AnimCharacter_MaskArray_MPD::props()
{
	static const mpd_KeyValue __size_metadata[] =
	{
		{ "visible", 7, "false", 5, 0, (float) 0 },
		{ "min", 3, "0", 1, 0, (float) 0 },
		{ "max", 3, "1000", 4, 1000, (float) 1000 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "__size", 6, { "int32_t", mpdt_Int32, 0 }, __size_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant AnimCharacter_MaskArray_MPD::getprop( Array<AnimCharacter::Mask> const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (int32_t const&) obj->size();
	default: return mpd_Variant();
	}
}
bool AnimCharacter_MaskArray_MPD::setprop( Array<AnimCharacter::Mask>* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->resize(val.get_int32()); return true;
	default: return false;
	}
}
mpd_Variant AnimCharacter_MaskArray_MPD::getindex( Array<AnimCharacter::Mask> const* obj, const mpd_Variant& key )
{
	return (AnimCharacter::Mask const&)(*(obj))[mpd_var_get<int32_t >(key)];
}
bool AnimCharacter_MaskArray_MPD::setindex( Array<AnimCharacter::Mask>* obj, const mpd_Variant& key, const mpd_Variant& val )
{
	(*(obj))[mpd_var_get<int32_t >(key)] = mpd_var_get<AnimCharacter::Mask >(val);
	return true;
}
const mpd_EnumValue* AnimCharacter_MaskArray_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void AnimCharacter_MaskArray_MPD::dump( MPD_STATICDUMP_ARGS(Array<AnimCharacter::Mask>) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct AnimCharacter_MaskArray\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( int32_t, __size, pdata->size() );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

const mpd_KeyValue* AnimCharacter_MPD::metadata()
{
	static const mpd_KeyValue data[] =
	{
		{ "label", 5, "Character", 9, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	return data;
}
const mpd_PropInfo* AnimCharacter_MPD::props()
{
	static const mpd_KeyValue mesh_metadata[] =
	{
		{ "label", 5, "Mesh", 4, 0, (float) 0 },
		{ "edit", 4, "mesh", 4, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_KeyValue masks_metadata[] =
	{
		{ "label", 5, "Masks", 5, 0, (float) 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};
	static const mpd_PropInfo data[] =
	{
		{ "mesh", 4, { "String", mpdt_Struct, String_MPD::inst() }, mesh_metadata },
		{ "masks", 5, { "AnimCharacter_MaskArray", mpdt_Struct, AnimCharacter_MaskArray_MPD::inst() }, masks_metadata },
		{ 0, 0, { 0, mpdt_None, 0 }, 0 },
	};
	return data;
}
mpd_Variant AnimCharacter_MPD::getprop( AnimCharacter const* obj, int prop )
{
	switch( prop )
	{
	case 0: return (String const&) obj->mesh;
	case 1: return (Array<AnimCharacter::Mask> const&) obj->masks;
	default: return mpd_Variant();
	}
}
bool AnimCharacter_MPD::setprop( AnimCharacter* obj, int prop, const mpd_Variant& val )
{
	switch( prop )
	{
	case 0: obj->mesh = mpd_var_get<String >(val); return true;
	case 1: obj->masks = mpd_var_get<Array<AnimCharacter::Mask> >(val); return true;
	default: return false;
	}
}
mpd_Variant AnimCharacter_MPD::getindex( AnimCharacter const*, const mpd_Variant& ){ return mpd_Variant(); }
bool AnimCharacter_MPD::setindex( AnimCharacter*, const mpd_Variant&, const mpd_Variant& ){ return false; }
const mpd_EnumValue* AnimCharacter_MPD::values(){ static const mpd_KeyValue kvnone = { 0, 0, 0, 0, 0, 0 }; static const mpd_EnumValue none = { 0, 0, 0, &kvnone }; return &none; }
void AnimCharacter_MPD::dump( MPD_STATICDUMP_ARGS(AnimCharacter) )
{
	MPD_DUMPDATA_USESTATICARGS;
	printf( "struct AnimCharacter\n" );
	MPD_DUMPLEV( 0 ); printf( "{\n" );
	if( level < limit )
	{
		MPD_DUMP_PROP( String, mesh, pdata->mesh );
		MPD_DUMP_PROP( Array<AnimCharacter::Mask>, masks, pdata->masks );
	}
	else
	{
		MPD_DUMPLEV( 1 ); printf( "...\n" );
	}
	MPD_DUMPLEV( 0 ); printf( "}" );
}

#endif
