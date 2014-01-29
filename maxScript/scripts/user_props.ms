fn setVisibleObjProps obj state =
(
	setUserProp obj "is_visible" state
)

fn setExportMeshObjProps obj state =
(
	setUserProp obj "export_mesh" state
)

fn setExportMaterialsObjProps obj state =
(
	setUserProp obj "export_materials" state
)

fn setExportPhysicsObjProps obj state =
(
	setUserProp obj "export_physics" state
	if (state == true) then
	(
		setUserProp obj "physics_shape" "CONCAVE_MESH"
		setUserProp obj "physics_static" "true"
		setUserProp obj "physics_kinematic" "false"
		setUserProp obj "col_type" "VISIBLE_GEOM"
		setUserProp obj "is_visible" "false"
		if ((nvpx.IsConvex $) == true) then setUserProp obj "physics_shape" "CONVEX_MESH"
	)
)

fn setRigidPlane obj =
(
	setUserProp obj "physics_shape" "PLANE"
)

fn setRigidBox obj =
(
	setUserProp obj "physics_shape" "BOX"
)

fn setRigidConvex obj =
(
	setUserProp obj "physics_shape" "CONVEX_MESH"
)

fn setRigidConcave obj =
(
	setUserProp obj "physics_shape" "CONCAVE_MESH"
)

fn setColShadowProps obj =
(
	setUserProp obj "col_type" "VISIBLE_GEOM"
)
	
fn setColAIProps obj =
(
	setUserProp obj "col_type" "INVISIBLE_GEOM"
)
	
fn setColForbiddenProps obj =
(
	setUserProp obj "col_type" "FORBIDDEN"
)

utility UPsPanel "UserProps" width:180 height:142
(
	button btn_empty "Restart Properties" width:140 align:#left
	button btn_reload "Reload Properties" width:140 align:#left

	
	group "Basics" (
		checkbox check_visible "Is Visible"
		checkbox check_mesh "Export Mesh"
		checkbox check_materials "Export Materials"
		checkbox check_physics "Export Physics"
		checkbox check_forward "Forward Rendering"
		checkbox check_additive "Additive Blending"
	)
	
-- 	group "Shader" (
-- 		listbox lb_shaders "" items:#("Basic","Mix","Lightmap", "Mix&Lightmap","No Light", "Moving Water", "Still Water") 
-- 		button btn_apply_shader "Apply Shader" width:140 align:#left
-- 	)
	
-- 	group "Rigidbody Type" (
-- 		button btn_rigid_plane "Plane" width:140 align:#left
-- 		button btn_rigid_box "Box" width:140 align:#left
-- 		button btn_rigid_convex "Convex" width:140 align:#left
-- 		button btn_rigid_concave "Concave" width:140 align:#left
-- 	)
	
	group "Collision Type" (
		button btn_col_shadow "Col. shadow" width:140 align:#left
		button btn_col_ia "Col. AI" width:140 align:#left
		button btn_col_forbidden "Col. forbidden" width:140 align:#left
	)
	
	group "Enemy character" (
		listbox lb_characters "" items:#("xu normal", "xu shield", "xu pointlight", "one") height:7
		edittext talk_to "Talk to:" width:140 height:16
		button btn_apply_character "Set Character" width:140 align:#left
	)
	
	group "Particles" (
		listbox lb_particles "" items:#("Fire", "Fire Smoke", "Fireflies","Fog", "Light Dust") height:7
		button btn_apply_particles "Set Particles" width:140 align:#left
	)
	
	group "Steps Sound" (
		listbox lb_steps_sounds "" items:#("Grass","Wood","Stone", "Gravel", "Outside") height:6
		button btn_apply_steps_sound "Set sound" width:140 align:#left
	)
	
	group "Sound track" (
		listbox lb_sounds "" items:#("Fuego","Agua","Grillos") height:6
		button btn_apply_sound "Set sound" width:140 align:#left
	)
	
	group "Other" (
		checkbox check_billboard "Is Billboard"
		checkbox check_no_cull "Disable Culling"
		checkbox check_before_outline "Render Before Outline"
	)
	
	on btn_reload pressed do
	(
		if $ != undefined then 
		(
			local o = $
			if classof o == ObjectSet then o = o[1]
				
			if (getUserProp o "is_visible" == true) then check_visible.state = true
			else check_visible.state = false
			
			if (getUserProp o "export_mesh" == true) then check_mesh.state = true
			else check_mesh.state = false
			
			if (getUserProp o "export_materials" == true) then check_materials.state = true
			else check_materials.state = false
			
			if (getUserProp o "export_physics" == true) then check_physics.state = true
			else check_physics.state = false
				
			if (getUserProp o "forward" == true) then check_forward.state = true
			else check_forward.state = false
			
			if (getUserProp o "additive" == true) then check_additive.state = true
			else check_additive.state = false
			
			if (getUserProp o "before_outline" == true) then check_before_outline.state = true
			else check_before_outline.state = false
		)

	)
	
	on UPsPanel open do
	(
	
	)
	
	on check_visible changed theState do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setVisibleObjProps obj theState)
		)
		else 
		( 
			setVisibleObjProps $ theState
		)
	)
	
	on check_mesh changed theState do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setExportMeshObjProps obj theState)
		)
		else 
		( 
			setExportMeshObjProps $ theState
		)
	)
	
	on check_materials changed theState do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setExportMaterialsObjProps obj theState)
		)
		else 
		(
			setExportMaterialsObjProps $ theState
		)
	)
	
	on check_physics changed theState do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setExportPhysicsObjProps obj theState)
		)
		else ( setExportPhysicsObjProps $ theState)
	)
	
-- 	on btn_apply_shader pressed do
-- 	(
-- 		if (lb_shaders.selected == "Basic")  then setUserProp $ "material_name" "tech_basic"
-- 		else if (lb_shaders.selected == "Mix") then setUserProp $ "material_name" "tech_mix"
-- 		else if (lb_shaders.selected == "Lightmap") then setUserProp $ "material_name" "tech_lightmap"
-- 		else if (lb_shaders.selected == "Mix&Lightmap") then setUserProp $ "material_name" "tech_mix_lightmap"
-- 		else if (lb_shaders.selected == "No Light") then setUserProp $ "material_name" "tech_basic"
-- 		else if (lb_shaders.selected == "Moving Water") then setUserProp $ "material_name" "tech_water"
-- 		else if (lb_shaders.selected == "Still Water") then setUserProp $ "material_name" "tech_water_still"
-- 	)
	
	on btn_apply_character pressed do
	(
		if $ == undefined then throw "there's nothing selected"
		if (lb_characters.selected == "xu normal")  then setUserProp $ "character" "xu"
		else if (lb_characters.selected == "xu shield") then setUserProp $ "character" "xu_shield"
		else if (lb_characters.selected == "xu pointlight") then setUserProp $ "character" "xu_farolillo"
		else if (lb_characters.selected == "one") then setUserProp $ "character" "one"
		
		setUserProp $ "talk_to" talk_to.text	
	)
	
	on btn_apply_particles pressed do
	(
		if $ == undefined then throw "there's nothing selected"
		if (lb_particles.selected == "Fire")  then setUserProp $ "particles" "fire"
		else if (lb_particles.selected == "Fireflies") then setUserProp $ "particles" "fireflies"
		else if (lb_particles.selected == "Fog") then setUserProp $ "particles" "fog"
		else if (lb_particles.selected == "Fire Smoke") then setUserProp $ "particles" "smoke_fire"
		else if (lb_particles.selected == "Light Dust") then setUserProp $ "particles" "light_dust"
	)
	
	on btn_apply_steps_sound pressed do
	(
		if $ == undefined then throw "there's nothing selected"
		if (lb_steps_sounds.selected == "Grass")  then setUserProp $ "steps" "grass"
		else if (lb_steps_sounds.selected == "Wood") then setUserProp $ "steps" "wood"
		else if (lb_steps_sounds.selected == "Stone") then setUserProp $ "steps" "stone"
		else if (lb_steps_sounds.selected == "Gravel") then setUserProp $ "steps" "gravel"
		else if (lb_steps_sounds.selected == "Outside") then setUserProp $ "steps" "outside"
	)
	
	on btn_apply_sound pressed do
	(
		if $ == undefined then throw "there's nothing selected"
		if (lb_sounds.selected == "Fuego")  then setUserProp $ "sound" "fuego"
		else if (lb_sounds.selected == "Agua") then setUserProp $ "sound" "agua"
		else if (lb_sounds.selected == "Grillos") then setUserProp $ "sound" "grillos"
	)
	
	on btn_visible_obj pressed do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setVisibleObjProps obj)
		)
		else ( setVisibleObjProps $)
	)
	
	on btn_invisible_obj pressed do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setInvisibleObjProps obj)
		)
		else ( setInvisibleObjProps $)
	)
	
	on btn_rigid_plane pressed do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setRigidPlane obj)
		)
		else ( setRigidPlane $)
	)
	
	on btn_rigid_box pressed do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setRigidBox obj)
		)
		else ( setRigidBox $)
	)
	
	on btn_rigid_convex pressed do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setRigidConvex obj)
		)
		else ( setRigidConvex $)
	)
	
	on btn_rigid_concave pressed do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setRigidConcave obj)
		)
		else ( setRigidConcave $)
	)

	on btn_col_shadow pressed do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setColShadowProps obj)
		)
		else ( setColShadowProps $)
	)
	
	on btn_col_ia pressed do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setColAIProps obj)
		)
		else ( setColAIProps $)
	)
	
	on btn_col_forbidden pressed do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setColForbiddenProps obj)
		)
		else ( setColForbiddenProps $)
	)
	
	on btn_empty pressed do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do 
			( 
				setUserPropBuffer obj "" 
			)
		)
		else 
		( 
			setUserPropBuffer $ "" 
		)
		
		check_visible.state = false
		check_mesh.state = false
		check_materials.state = false
		check_physics.state = false
		check_billboard.state = false
		check_no_cull.state = false
		check_alpha.state = false
		check_before_outline = false;
	)
	
	on check_billboard changed theState do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setUserProp obj "billboard" theState)
		)
		else ( setUserProp $ "billboard" theState)
	)
	
	on check_no_cull changed theState do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setUserProp obj "no_cull" theState)
		)
		else ( setUserProp $ "no_cull" theState)
	)
	
	on check_forward changed theState do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setUserProp obj "forward" theState)
		)
		else ( setUserProp $ "forward" theState)
	)

	on check_additive changed theState do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setUserProp obj "additive" theState)
		)
		else ( setUserProp $ "additive" theState)
	)
	
	on check_before_outline changed theState do
	(
		if $ == undefined then throw "there's nothing selected"
		
		if classof given_obj == ObjectSet then
		(
			for obj in $ do ( setUserProp obj "before_outline" theState)
		)
		else ( setUserProp $ "before_outline" theState)
	)

)
