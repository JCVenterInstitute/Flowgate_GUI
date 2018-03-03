# Flowgate_GUI

mockups, sourcees and docker images for FlowGate’s customized clinical and/or translational research GUI portal

runing the webapp directly or via docker image
install grails 3.2.9 (follow instructions at visit http://www.grails.org/download.html and install version 3.2.9)
check grails version by typing

grails -v  (Terminal/Console[ix/ux] or Cmd[Wdos])

expected output
| Grails Version: 3.2.9
| Groovy Version: ....
| JVM Version: .....

clone repository (assuming git installed)

git clone https://github.com/JCVenterInstitute/Flowgate_GUI.git

change into cloned directory

cd Flowgate_GUI

change into subfolder grails_webapp

cd grails_webapp

run application directly by executing the command:

grails run

expected output
Grails application running at http://localhost:8585/flowgate in environment: ....
and locate your internet browser to: http://localhost:8585/flowgate/ after the app started when finished initialization

the app can be closed using the ctrl + c key combination

to create a docker image:  (you must be located in the grails_webapp folder)
execute command: 
./gradlew assemble buildImage
