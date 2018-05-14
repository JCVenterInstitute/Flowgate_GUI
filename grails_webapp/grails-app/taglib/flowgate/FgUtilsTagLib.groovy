package flowgate

import org.jsoup.Jsoup
import org.jsoup.nodes.Document
import org.jsoup.nodes.Element
import org.jsoup.nodes.Node
import org.jsoup.nodes.TextNode

class FgUtilsTagLib {

    static namespace = 'fg'
    static defaultEncodeAs = [taglib:'none']
//    static defaultEncodeAs = [taglib:'html']
    //static encodeAsForTags = [tagName: [taglib:'html'], otherTagName: [taglib:'none']]

    def utilsService

    /** Renders a html file from gp-server
     * used for rendering pipeline report results
     */
    def render = {attrs ->
        AnalysisServer analysisServer = AnalysisServer.get(attrs?.analysisServerId)
        if(attrs.href && attrs.href!='' && analysisServer) {
            def fileUrl = new URL(attrs.href)
            def connection = fileUrl.openConnection()
            connection.setRequestProperty ("Authorization", utilsService.authHeader(analysisServer.userName, analysisServer.userPw))
            def dataStream = connection.inputStream
//            String outPut
//            outPut = dataStream.text
//            Document doc = Jsoup.connect(attrs.href).header("Authorization", utilsService.authHeader(analysisServer.userName, analysisServer.userPw)).get()
//            Element element = doc.getElementById("notebook")
//            outPut = element?.text()
//            outPut = doc?.text()
//            out << outPut
            out << dataStream
        }
        else {
            out << 'Error: no report file found!'
        }
    }

    /* */
    def render2 = {attrs ->
        AnalysisServer analysisServer = AnalysisServer.get(attrs?.analysisServerId)
        if(attrs.href && attrs.href!='' && analysisServer) {
            Document document = Jsoup.connect(attrs.href).header("Authorization", utilsService.authHeader(analysisServer.userName, analysisServer.userPw)).get()
            Element element = document.getElementById("notebook")
            out << """
                     <style>
                     a.anchor-link:link {
                        text-decoration: none;
                        padding: 0px 20px;
                        visibility: hidden;
                    }
                    </style>   
                    """
            out << element
        }
        else {
            out << 'Error: no report file found!'
        }
    }
    /* */

    def renderHtmlFile = {attrs ->
        def htmlFile = grailsApplication.mainContext.getResource("classpath:resources/$attrs.fileName").file
        out << htmlFile.text
    }

}
