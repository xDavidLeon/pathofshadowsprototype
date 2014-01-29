clearListener()

fn findRootOfSkeleton= (
	for i in $* do (
		if classof i == Biped_Object then (
			if i.parent == undefined  then (
				return i
			)
		)
	)
	throw "Can't find any root of skeleton in the scene"
)

fn findBonesToExport = (
	
	local all_bones = #()
	for i in $* do (
		if classof i == Biped_Object then (
			format "Adding % % \n" (classof i) i.name 
			append all_bones i
		)
	)
	return all_bones
)

fn isSkeletonMesh obj = (
	for i in obj.modifiers do (
		if classof i == Skin then return true
		if classof i == Physique then return true
	)
	return false
)
	

fn findMeshesOfSkeleton = (
	local all_meshes = #()
	
	for i in $* do (
		if classof i == PolyMeshObject or \
		   classof i == Editable_Mesh or \
		   classof i == Editable_Poly then (
			if isSkeletonMesh i then (
				format "Adding mesh % % \n" (classof i) i.name 
				append all_meshes i
			)
		)
	)
	
	
	return all_meshes
)

struct exportModelParams (model_name, base_path, csf, anim_name )
	
fn getExportModelParams = (
	if mcv_data_path == undefined then throw "mcv_data_path is not defined!"

	local root_bone= findRootOfSkeleton()

	local model_name = $.name
	local base_path = mcv_data_path + "characters/" + model_name + "/"
	if (doesFileExist base_path == false) then (
		makeDir base_path
	)
	local csf_filename = base_path + model_name + ".csf"
	--local anim_name = getUserProp root_bone "anim_name"
	local anim_name = (filterString maxfilename ".")[1] --nombre del archivo, sin extension
	
	return exportModelParams model_name:model_name base_path:base_path csf:csf_filename anim_name:anim_name
)


fn exportModelSkel = (
	
	local p = getExportModelParams()
	
	local root_bone= findRootOfSkeleton()
	local was_in_figure_mode = root_bone.controller.figureMode
	if was_in_figure_mode == false then (
		root_bone.controller.figureMode = true
	)
	
	-- Export skel
	local bones_to_export = findBonesToExport()
	format "Exporting skel to %\n" p.csf
	format "Bones are:\n"
	print bones_to_export
	local rc = exportCalSkel p.csf bones_to_export false
	if rc != 0 then throw ( "exportCalSkel failed with code " + ( rc as string ))
	
	if was_in_figure_mode == false then (
		root_bone.controller.figureMode = false
	)
)
		
fn exportModelMeshes = (

	local p = getExportModelParams()

	-- Pasar a figure mode si es necesario
	local was_in_figure_mode = root_bone.controller.figureMode
	if was_in_figure_mode == false then (
		root_bone.controller.figureMode = true
	)
	
	-- Export candidate meshes
	local meshes_to_export = findMeshesOfSkeleton()
	for m in meshes_to_export do (
		local cmf_filename = p.base_path + m.name + ".cmf"
		format "Exporting mesh % to %\n"m.name cmf_filename
		local rc = exportCalMesh cmf_filename \
							   p.csf \
							   m \
							   4 \
							   0.01 \
							   false \
							   false
		if rc != 0 then throw ( "exportCalMesh failed with code " + ( rc as string ))
	)
	
	-- Restore figure mode if it was in figure mode
	if was_in_figure_mode == false then (
		root_bone.controller.figureMode = false
	)	
)

fn exportAnimation = (
	
	-- Quitar el figure mode si es necesario
	local root_bone= findRootOfSkeleton()
	local was_in_figure_mode = root_bone.controller.figureMode
	if was_in_figure_mode == true then (
		root_bone.controller.figureMode = false
	)	
	
	local p = getExportModelParams()
	if p.anim_name == undefined then throw "Pon el nombre de la animacion en el panel 'Object Properties => User Defined' del root bone. Por ejemplo: anim_name=idle"
	local caf_filename = p.base_path + p.anim_name + ".caf"
	local bones_to_export = findBonesToExport()
	local beg_frame = (animationRange.start.frame as integer)
	local end_frame = (animationRange.end.frame as integer)
	format "Exporting anim % from % to % fps % to % \n" \
	    p.anim_name beg_frame end_frame frameRate caf_filename
	local rc = exportCalAnim caf_filename \
	                       p.csf \
						   bones_to_export \
	                       beg_frame \
	                       end_frame \
						   0 \
	                       frameRate
	if rc != 0 then throw ( "exportCalAnim failed with code " + ( rc as string ))
)

fn exportXMLAnimationTrack obj = ( 
	local beg_frame = (animationRange.start.frame as integer)
	local end_frame = (animationRange.end.frame as integer)
	--local fs = createFile filaneme "w+" 
	--fs = listener
	--for i = beg_frame to end_frame do (
	--	local loc = at time i obj.position
	--	format "<at time=\"%\" loc=\"%\"/>\n"  i loc
	--	)
	--close fs
	
	local filename = "prueba"		
	local full_path = mcv_data_path + "characters\\" + filename	+ ".xml"
	local myFile = openFile full_path mode:"wt"
 	if myFile == undefined then (
 		throw ( "Can't create output animation track xml " + full_path )
 	)
	
	format "<animation>\n" to:myFile
	
	for i = beg_frame to end_frame do (
		local loc = at time i obj.position
		format "\t<at time=\"%\" x=\"%\" y=\"%\" z=\"%\" />\n"  i loc.x loc.y loc.z to:myFile
	)
	
	format "</animation>\n" to:myFile
	
 	close myFile
	format "Exported: %\n" filename
)



--exportModelSkel()
--exportModelMeshes()




--exportXMLAnimationTrack $

--exportAnimation()
--b = findBonesToExport()
--exportModel()
--print b