
[[pass]] any_light_pass
	Enabled False
	AlphaBlend True
	SrcBlend SrcAlpha
	DstBlend One
	ZWriteEnable False
[[endpass]]

[[pass]] shadow_pass
	Order 1500
	DepthBias 1e-5
	SlopeDepthBias 0.5
	CullMode None
[[endpass]]

[[pass]] base_solid_pass
	Order 2500
[[endpass]]

[[pass]] point_light_solid_pass
	Inherit any_light_pass
	Enabled True
	Order 3500
[[endpass]]

[[pass]] spot_light_solid_pass
	Inherit any_light_pass
	Enabled True
	Order 4500
[[endpass]]

[[pass]] base_decal_pass
	Order 5500
	DepthBias 1e-5
	SlopeDepthBias 0.5
	ZWriteEnable False
	AlphaBlend True
	SrcBlend SrcAlpha
	DstBlend InvSrcAlpha
[[endpass]]

[[pass]] point_light_decal_pass
	Inherit any_light_pass
	Enabled True
	Order 6500
[[endpass]]

[[pass]] spot_light_decal_pass
	Inherit any_light_pass
	Enabled True
	Order 7500
[[endpass]]

[[pass]] base_transparent_pass
	Order 8500
	ZWriteEnable False
	AlphaBlend True
	SrcBlend SrcAlpha
	DstBlend InvSrcAlpha
[[endpass]]

[[pass]] point_light_transparent_pass
	Inherit any_light_pass
	Enabled True
	Order 9500
[[endpass]]

[[pass]] spot_light_transparent_pass
	Inherit any_light_pass
	Enabled True
	Order 10500
[[endpass]]
