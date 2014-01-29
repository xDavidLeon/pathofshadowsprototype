Para poder exportar cosas con el max, hacer lo siguiente:

1) Copiar el archivo de plugin al directorio:
--> C:\Program Files\Autodesk\<carpeta del max>\plugins\
--> El nombre del archivo de plugin es "mcv(versiónMax).dlx".

2) Copiar el script "mcv.ms" (en ./Startup) a la carpeta de Starup de max
--> C:\Program Files\Autodesk\<carpeta del max>\Scripts\Startup\

3) Crear el archivo "mcv.ini" en el directorio:
--> C:\Program Files\Autodesk\<carpeta del max>\Scripts\
Con el siguiente contenido (uso <<< >>> para marcar el inicio y el fin del archivo; no hay que escribirlo en el .ini):
<<<

[Paths]
mesh_path=***Copiar aqui directorio donde queramos que vaya el output (en mi caso: code(carpeta del repositorio)\data\ )***
scripts_path=***Copiar aqui directorio donde se encuentren el resto de scripts. Como estan en el repositorio, poned: code(carpeta del repositorio)\maxScript\scripts\ ***

>>>

==> Al abrir el MaxScript listener de max debería salir el mensajito: "MCV scripts loaded". Si no sale es que el .ini está mal
==> Para exportar escena, abrir "export_scene.ms", escogen nombre de archivo abajo del todo, y darle a Crtl+E