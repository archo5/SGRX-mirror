[[pass]] shadow_pass
	DepthBias 1e-5f
	SlopeDepthBias 0.5f
	CullMode None
[[endpass]]
[[pass]] any_light_pass
	Enabled False
	AlphaBlend True
	SrcBlend SrcAlpha
	DstBlend One
	ZWriteEnable False
[[endpass]]
[[pass]] point_light_pass
	Inherits any_light_pass
	Enabled True
[[endpass]]
[[pass]] spot_light_pass
	Inherits any_light_pass
	Enabled True
[[endpass]]
