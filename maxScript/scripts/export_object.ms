--clearListener()

-- Comprobar que el nombre del fichero no tiene espacios 
-- no tiene caracteres 'raros' [a-z0-9_] (deberes)
-- Exportar el AABB (deberes)
-- + El listado de materiales ( en clase)
-- + Normales ( ya esta)
-- + Optimizar


function calculateTangentArray all_vtxs all_idxs = (

	local vertexCount = all_vtxs.count
	local triangleCount = all_idxs.count / 3
	-- local vertex = 
	-- normal salen del vertex
	
	local	tan1 = #()
	local	tan2 = #()
	
	-- Reset contents
	for i = 1 to vertexCount do (
		tan1[ i ] = [0,0,0]
		tan2[ i ] = [0,0,0]
	)
	
	--format "We have % triangles\n" triangleCount
    
	for a = 0 to triangleCount-1 do (
		
		--format "checking vtx %/%\n" a (triangleCount-1)
		
		local i1 = all_idxs[ a*3 + 1 ]
		if i1 == undefined then throw ("invalid i1 for a " + a)
        local i2 = all_idxs[ a*3 + 2 ]
		if i2 == undefined then throw "invalid i2"
        local i3 = all_idxs[ a*3 + 3 ]
		if i3 == undefined then throw "invalid i3"
		
		local vtx1 = all_vtxs[ 1+ i1 ]
		local vtx2 = all_vtxs[ 1+ i2 ]
		local vtx3 = all_vtxs[ 1+ i3 ]

		local v1 = [ vtx1[1], vtx1[2], vtx1[3] ]
		local v2 = [ vtx2[1], vtx2[2], vtx2[3] ]
		local v3 = [ vtx3[1], vtx3[2], vtx3[3] ]
        
		local w1 = [ vtx1[7], vtx1[8] ]
		local w2 = [ vtx2[7], vtx2[8] ]
		local w3 = [ vtx3[7], vtx3[8] ]
        
        local x1 = v2.x - v1.x;
        local x2 = v3.x - v1.x;
        local y1 = v2.y - v1.y;
        local y2 = v3.y - v1.y;
        local z1 = v2.z - v1.z;
        local z2 = v3.z - v1.z;
        
        local s1 = w2.x - w1.x;
        local s2 = w3.x - w1.x;
        local t1 = w2.y - w1.y;
        local t2 = w3.y - w1.y;
        
        local r = 1.0 / (s1 * t2 - s2 * t1);
        local sdir = [(t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r]
        local tdir = [(s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r]
        
        tan1[1+i1] += sdir;
        tan1[1+i2] += sdir;
        tan1[1+i3] += sdir;
        
        tan2[1+i1] += tdir;
        tan2[1+i2] += tdir;
        tan2[1+i3] += tdir;
	)
    
    for a = 1 to vertexCount do (
		local vtx = all_vtxs[ a ]
		
		local n = [ vtx[4], vtx[5], vtx[6] ]
		local t = tan1[a];
        
        -- Gram-Schmidt orthogonalize
		local dot_nt = dot n t
        local tangent_a = normalize(t - n * dot_nt)
        
        -- Calculate handedness
		local cross_nt = cross n t
		local dot_cross = dot cross_nt tan2[a]
        local tangent_a_w = 1
		if dot_cross < 0.0 then tangent_a_w = -1
			
		join all_vtxs[a] #(tangent_a[1], tangent_a[2], tangent_a[3], tangent_a_w)
		
		--format " all_vtxs % is now %\n" a all_vtxs[a]
	)
	
)


-- Devuelve la posicion de new_vtx en el array all_vtxs si existe
-- Si no existe, lo registra
-- Devuelve el indice en base-0!!!
fn findUniqueIndex all_vtxs new_vtx = (
	-- Buscar usando c++
	local idx = findIndexOfFloats new_vtx
	-- Si el indice que me dan coincide con el tamaño del # de vertices que 
	-- tengo actualmente guardado, significa que es nuevo
	if idx == all_vtxs.count then (
		append all_vtxs new_vtx
	)
	return idx
)

fn exportMesh given_obj &log_str = ( 
	if given_obj == undefined then throw "exportMesh requires an object to export"
	local obj = given_obj
	
	if classof given_obj == ObjectSet then throw "Choose a single object to export"
	
	if not canConvertTo given_obj Editable_Mesh  then throw ( "The object "+ given_obj.name + " can't be exported as mesh")
	
	-- Convert obj to editable if it´s not already a mesh
	if classof given_obj != Editable_Mesh then (
		obj = copy given_obj
		convertToMesh obj 
	)
	print ("objeto actual: "+obj.name)
	
	-- Borrar el indice en c++, pq esta malla es nueva
	clearIndexOfFloats()
	
	-- Export obj
	local nfaces = getNumFaces obj
	
	local mat_ids = get_materials_used_by_obj obj
	
	local max2local = inverse obj.transform
	local axis_x_90 = (quat 90 [1,0,0] ) as matrix3
	max2local = max2local * axis_x_90
	
	-- The same matrix, but without the translation, to rotate the normals
	local max2local_rot = max2local.rotationpart 
	
		-- AABB
	--local aabb = nodeLocalBoundingBox obj
-- 	local minpos = obj.min
-- 	local maxpos = obj.max
-- 	minpos = minpos * max2local
-- 	maxpos = maxpos * max2local
	--local minpos = obj.min * max2local
	--local maxpos = obj.max * max2local
	
	-- An emtpy array to hold all vertexs
	local all_vtxs = #()
	local all_idxs = #()
	local groups_info = #()
	local first_index = 0
	
	-- For all faces
	for faces_of_mat_id in mat_ids do (
		if faces_of_mat_id != undefined then (
			
			local face_id
			for face_id in faces_of_mat_id do (
				-- The face [3 indices] to access the geometry positions
				local face = getFace obj face_id
				
				-- Get the 3 normals, one for each vertex of the face
				local normals = meshop.getFaceRNormals obj face_id
				
				-- For the 3 vertexs of each face
				local vid
				for vid = 1 to 3 do (
					-- Access the real positions in 3D
					local world_position = getVert obj face[ vid ]
					local local_position = world_position * max2local
					-- Convert the normal to dx coordinate system
					local normal = normals[ vid ] * max2local_rot			
					
					-- Generate all the info for my vertex
					local v = #( local_position.x, local_position.y, local_position.z \
						            , normal.x, normal.y, normal.z
						)
					
					-- Find the unique idx
					local idx = findUniqueIndex all_vtxs v
					append all_idxs idx
				)
			)
			
			-- Preparar la informacion del grupo/submaterial
			local g = #( first_index, faces_of_mat_id.count*3 )
			first_index = first_index + faces_of_mat_id.count*3
			append groups_info g
			
		)
	)
	
	-- De momento solo hay soporte para 65K vertices
	if all_idxs.count > 65535 then throw ("El objeto " + given_obj.name + " tiene demasiados vertices (indices:"+(all_idxs.count as string)+"). No se puede exportar!" )
	
	local filename = mcv_data_path + "meshes\\" + subfileName + "\\" + given_obj.name + ".mesh"

	--format "Exporting mesh % with % unique vertexs and % indices. Lightmaps: %\n" \
	--    given_obj.name all_vtxs.count all_idxs.count has_lightmap
	
	log_str = log_str + "- Exporting mesh "+given_obj.name+" with "+(all_vtxs.count as string)+" unique vertexs and "+(all_idxs.count as string)+" indices.\n"
	
	-- Open output file
	local f = fopen filename "wb"
	if f == undefined then throw ( "Can't create output file: " + filename )
	
	-- Save Header
	local mesh_magic = 0x2233EE11
	local current_version = 5
	local vertexs_type = 1099			-- position + normal
	local bytes_per_vertex = all_vtxs[1].count * 4 -- sizeof(float) = 4
	local bytes_per_index = 4 		-- sizeof(int) = 4
	local primitive_type = 4				-- == D3DPT_TRIANGLELIST
	local nindices = all_idxs.count
	local flags      = 0
	
	WriteLong f mesh_magic	    -- Magic
	WriteLong f current_version 
	WriteLong f nfaces
	WriteLong f all_vtxs.count
	WriteLong f vertexs_type
	WriteLong f bytes_per_vertex
	WriteLong f bytes_per_index
	WriteLong f primitive_type
	WriteLong f nindices
	WriteLong f flags
	WriteLong f groups_info.count		-- Cuantos submateriales tiene
		
	-- TODO
	-- nmateriales y rango de cada material
	-- ... more things go here. Keep in sync with c++ reader
	WriteLong f mesh_magic	    -- Magic at the end of the header
	
	-- Group infos
	for g in groups_info do (
		writeLong f g[1]			-- Indice inicial donde empieza el grupo
		writeLong f g[2]			-- # de indices de este grupo
		writeLong f 0				-- # de bones 0
	)
	
	-- Save vertexs
	for v in all_vtxs do (		-- For each vertex
		for c in v do (			-- For each component of the vertex
			WriteFloat f c			-- Save a float
		)
	)
	
	-- Save indices
	for i in all_idxs do (		-- For each vertex
		WriteLong f i #unsigned		-- Save a short
	)	
	
-- 	-- Save Bounding Box
-- 	WriteFloat f minpos[1]
-- 	WriteFloat f minpos[2]
-- 	WriteFloat f minpos[3]
-- 	WriteFloat f maxpos[1]
-- 	WriteFloat f maxpos[2]
-- 	WriteFloat f maxpos[3]
	
	WriteLong f mesh_magic	    -- Magic footer
	fclose f
	
	-- Check if we did a remove the copy
	if obj != given_obj then delete obj
	
	return mat_ids
)


fn exportMeshWithUVs given_obj  &log_str = ( 
	if given_obj == undefined then throw "exportMeshWithUVs requires an object to export"
	local obj = given_obj
	
	if classof given_obj == ObjectSet then throw "Choose a single object to export"
	
	if not canConvertTo given_obj Editable_Mesh  then throw ( "The object "+ given_obj.name + " can't be exported as mesh")
	
	-- Convert obj to editable if it´s not already a mesh
	if classof given_obj != Editable_Mesh then (
		obj = copy given_obj
		convertToMesh obj 
	)
	
	-- Borrar el indice en c++, pq esta malla es nueva
	clearIndexOfFloats()
	
	-- Export obj
	local nfaces = getNumFaces obj
	
	local mat_ids = get_materials_used_by_obj obj
	
	local max2local = inverse obj.transform
	local axis_x_90 = (quat 90 [1,0,0] ) as matrix3
	max2local = max2local * axis_x_90
	
	-- The same matrix, but without the translation, to rotate the normals
	local max2local_rot = max2local.rotationpart 
	
		-- AABB
	--local aabb = nodeLocalBoundingBox obj
-- 	local minpos = obj.min
-- 	local maxpos = obj.max
-- 	minpos = minpos * max2local
-- 	maxpos = maxpos * max2local
	--local minpos = obj.min * max2local
	--local maxpos = obj.max * max2local
	
	-- An emtpy array to hold all vertexs
	local all_vtxs = #()
	local all_idxs = #()
	
	local channelA = 1
	local channelB = 3
	local channelC = 4
		
	local has_difffusemap = meshop.getMapSupport obj channelA
	
	if has_difffusemap == false then ( 
		throw ("Object has no UV data: " + given_obj.name)
		--format ("Object has no UV data: " + obj.name + ". Ignoring...\n")
		if obj != given_obj then delete obj
		return false
	)
	
	print ("objeto actual: "+obj.name)
	
	local has_mixmap = meshop.getMapSupport obj channelB
	local has_lightmap = meshop.getMapSupport obj channelC
	
	log_str += "Mesh " + obj.name + " has "
	if (has_diffusemap == true) then log_str += "DiffuseMap "
	if (has_mixmap == true) then log_str += "MixMap "
	if (has_lightmap == true) then log_str += "Lightmap "
	
	local groups_info = #()
	local first_index = 0
	
	-- For all faces
	for faces_of_mat_id in mat_ids do (
		if faces_of_mat_id != undefined then (
			
			local face_id
			for face_id in faces_of_mat_id do (
				-- The face [3 indices] to access the geometry positions
				local face = getFace obj face_id
				-- The face [3 indices] to access the uv coords
				local tfaceA = meshop.getMapFace obj channelA face_id
				local tfaceB = undefined 
				if has_mixmap == true then
				(
					tfaceB = meshop.getMapFace obj channelB face_id
				)
				local tfaceC = undefined 
				if has_lightmap == true then
				(
					tfaceC = meshop.getMapFace obj channelC face_id
				)
				
				-- Get the 3 normals, one for each vertex of the face
				local normals = meshop.getFaceRNormals obj face_id
				
				-- For the 3 vertexs of each face
				local vid
				for vid = 1 to 3 do (
					-- Access the real positions in 3D
					local world_position = getVert obj face[ vid ]
					local local_position = world_position * max2local
					-- Convert the normal to dx coordinate system
					local normal = normals[ vid ] * max2local_rot			
					
					-- Access the real uv's values of the diffusemap
					local uvA = meshop.getMapVert obj channelA tfaceA[ vid ]
					
					-- Access the real uv's values of the lightmap
					local uvB = undefined
					if has_mixmap == true then (
						uvB = meshop.getMapVert obj channelB tfaceB[ vid ]
					)
					local uvC = undefined
					if has_lightmap == true then (
						uvC = meshop.getMapVert obj channelC tfaceC[ vid ]
					)

					-- Generate all the info for my vertex
					local v = #( local_position.x, local_position.y, local_position.z \
						            , normal.x, normal.y, normal.z \
						            , uvA.x, 1-uvA.y \
						            --,  4 coords para el tangent vector + w
						)
					if has_mixmap == true then (
						append v uvB.x
						append v (1-uvB.y)
					)
					if has_lightmap == true then (
						append v uvC.x
						append v (1-uvC.y)
					)
					
					-- Find the unique idx
					local idx = findUniqueIndex all_vtxs v
					append all_idxs idx
				)
			)
			
			-- Preparar la informacion del grupo/submaterial
			local g = #( first_index, faces_of_mat_id.count*3 )
			first_index = first_index + faces_of_mat_id.count*3
			append groups_info g
			
		)
	)
	
	-- De momento solo hay soporte para 65K vertices
	if all_idxs.count > 65535 then throw ("El objeto " + given_obj.name + " tiene demasiados vertices (indices:"+(all_idxs.count as string)+"). No se puede exportar!" )
	
	local filename = mcv_data_path + "meshes\\" + subfileName + "\\" + given_obj.name + ".mesh"

	--format "Exporting mesh % with % unique vertexs and % indices. Lightmaps: %\n" \
	--    given_obj.name all_vtxs.count all_idxs.count has_lightmap
	
	log_str = log_str + "- Exporting mesh (with UV) "+given_obj.name+" with "+(all_vtxs.count as string)+" unique vertexs and "+(all_idxs.count as string)+" indices.\n"
	
	-- Open output file
	local f = fopen filename "wb"
	if f == undefined then throw ( "Can't create output file: " + filename )
	
	calculateTangentArray all_vtxs all_idxs
	
	-- Save Header
	local mesh_magic = 0x2233EE11
	local current_version = 4
	local vertexs_type = 1010			-- position + normal + uv + tangent
	if has_mixmap == true then vertexs_type = 1011 -- position + normal + 2uv + tangent
	if has_lightmap == true then vertexs_type = 1011 -- position + normal + 2uv + tangent
	if (has_mixmap == true and has_lightmap == true) then 
	(
		vertexs_type = 1012
		log_str = log_str + "- Vertex Type 1012\n"
	)
	local bytes_per_vertex = all_vtxs[1].count * 4 -- sizeof(float) = 4
	local bytes_per_index = 4 		-- sizeof(int) = 4
	local primitive_type = 4				-- == D3DPT_TRIANGLELIST
	local nindices = all_idxs.count
	local flags      = 0
	

	
	
	WriteLong f mesh_magic	    -- Magic
	WriteLong f current_version 
	WriteLong f nfaces
	WriteLong f all_vtxs.count
	WriteLong f vertexs_type
	WriteLong f bytes_per_vertex
	WriteLong f bytes_per_index
	WriteLong f primitive_type
	WriteLong f nindices
	WriteLong f flags
	WriteLong f groups_info.count		-- Cuantos submateriales tiene
		
	-- TODO
	-- nmateriales y rango de cada material
	-- ... more things go here. Keep in sync with c++ reader
	WriteLong f mesh_magic	    -- Magic at the end of the header
	
	-- Group infos
	for g in groups_info do (
		writeLong f g[1]			-- Indice inicial donde empieza el grupo
		writeLong f g[2]			-- # de indices de este grupo
		writeLong f 0				-- # de bones 0
	)
	
	-- Save vertexs
	for v in all_vtxs do (		-- For each vertex
		for c in v do (			-- For each component of the vertex
			WriteFloat f c			-- Save a float
		)
	)
	
	-- Save indices
	for i in all_idxs do (		-- For each vertex
		WriteLong f i #unsigned		-- Save a short
	)	
	
-- 	-- Save Bounding Box
-- 	WriteFloat f minpos[1]
-- 	WriteFloat f minpos[2]
-- 	WriteFloat f minpos[3]
-- 	WriteFloat f maxpos[1]
-- 	WriteFloat f maxpos[2]
-- 	WriteFloat f maxpos[3]
	
	WriteLong f mesh_magic	    -- Magic footer
	fclose f
	
	-- Check if we did a remove the copy
	if obj != given_obj then delete obj
	
	return mat_ids
)


--Creates xml file with the object's info
fn exportObject given_obj = ( 
	--Pasamos el nombre a lower case
	given_obj.name = toLower given_obj.name
	
	--The content of the file
	local fs = ""
	local full_path = mcv_data_path + "objects/" + given_obj.name + ".xml"
	local tempStream = "" as stringstream
	
	format "	<obj name=\"%\" mesh=\"%\">\n" given_obj.name given_obj.name to:tempStream
	
	fs += tempStream
	
	-- La lista de caras de cada material usado
	local mat_ids = #()
	--Exportamos mesh
	mat_ids = exportMeshWithUVs given_obj
	--Exportamos propiedades del objeto
	exportUserProperties given_obj &fs
	--Exportamos materiales
	exportMaterials given_obj mat_ids &fs 

	-- Tag de fin de obj
	tempStream = "" as stringstream
	format "	</obj>\n" to:tempStream
	
	fs += tempStream

	
	-- Crear el fichero en format texto (xml). Se borra si existe..
 	local myFile = openFile full_path mode:"wt"
 	if myFile == undefined then (
 		throw ( "Can't create output scene xml " + full_path )
 	)
	format fs to:myFile
	
 	close myFile
)

--Exports all selected objects separately
fn exportObjects given_objs = (
	
	if given_objs == undefined then throw "exportObjects requires at least one object to export"
	
	for obj in $ do
	(
		--Sólo podemos exportar objetos que sean EditableMesh o puedan convertirse en una
		if canConvertTo obj Editable_Mesh  then (
			format "Exportando objeto: %\n" obj.name
			exportObject obj
			format "\n"
		)
		else (format "WARNING: % is not a valid object to export. Ignoring..." obj.name)
	)
	
)

--exportObject $

--exportMeshWithUVs $