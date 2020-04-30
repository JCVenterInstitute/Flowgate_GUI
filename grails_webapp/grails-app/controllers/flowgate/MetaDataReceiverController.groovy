package flowgate

import grails.converters.JSON
import groovy.json.JsonSlurper

class MetaDataReceiverController {

    def index() {
//        def i=0
        Map fcsParams
        def req = request
        def jsnSlurper = new JsonSlurper()

        println "${params}"
        println "${req?.JSON}"
        if(req?.JSON){
            println "path ${req?.JSON?.path}"
//            def fileNameMatcher = req?.JSON?.path =~ /([^\/]+$)/
            def fileName = req?.JSON?.path - ~/([^\/]+$)/
//            String fileName = fileNameMatcher[0]?.first()
            println "fN: ${fileName}"

            Integer userIndex = req?.JSON?.path?.indexOf("/uploads/tmp/")
            if(userIndex!=0){
                println "uI: ${userIndex.toString()}"
//                def userNameMatcher = req?.JSON?.path?.minus(req?.JSON?.path?.substring(userIndex)) =~ /([^\/]+$)/
                String userPath = req?.JSON?.path - (req?.JSON?.path?.substring(userIndex))
                println "uP: ${userPath}"
                def userName = userPath - ~/^(.*[\\\/])/
//                def userName = (req?.JSON?.path - (req?.JSON?.path?.substring(userIndex))) - ~/([^\/]+$)/
//                String userName = userNameMatcher[0]?.first()
                println "uN: ${userName}"
            }
            else {
                println "user not found"
            }
            req?.JSON.each {
//                println "k: ${it.key}         v: ${it.value}"
            }
        }
        println "\n"
        render status: 200, "metadata received!\n"
    }
}
