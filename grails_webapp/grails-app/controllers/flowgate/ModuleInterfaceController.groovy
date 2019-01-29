package flowgate

import grails.converters.JSON

class ModuleInterfaceController {

    def assetResourceLocator

    def index() { }

//    https://stackoverflow.com/questions/36457869/grails-3-get-asset-path-in-service
//    assetResourceLocator.findAssetForURI('schemas/resource.json').getURI()

//    assetResourceLocator?.findAssetForURI('myFolder/placeholder.jpg')?.getInputStream()?.bytes


//    http://www.damirscorner.com/blog/posts/20160313-AccessingApplicationFilesFromCodeInGrails.html


//    def writeConfigTextfile(String fileName, def fdata){
    def writeConfigTextfile(){
        println "writing file ${params?.fileName}  ${params?.fdata}"

        def servResponse = [
            'success': true,
            'status': 'OK',
            'msg': 'config text file written to server'
        ]
        render servResponse as JSON

    }



}
