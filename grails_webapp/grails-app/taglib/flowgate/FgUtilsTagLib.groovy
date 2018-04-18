package flowgate

<<<<<<< HEAD
<<<<<<< HEAD
//import grails.io.IOUtils
import org.apache.commons.io.IOUtils

//import org.grails.encoder.Encoder
//import org.grails.taglib.TagOutput
//import org.grails.taglib.encoder.OutputContextLookupHelper
//import org.springframework.web.servlet.support.RequestContextUtils
=======
import org.jsoup.Jsoup
import org.jsoup.nodes.Document
import org.jsoup.nodes.Element
import org.jsoup.nodes.Node
import org.jsoup.nodes.TextNode

>>>>>>> master

class FgUtilsTagLib {

    static namespace = 'fg'
<<<<<<< HEAD
//    static defaultEncodeAs = [taglib:'html']
    static defaultEncodeAs = [taglib:'none']
    //static encodeAsForTags = [tagName: [taglib:'html'], otherTagName: [taglib:'none']]

    def utilsService

    /** Renders a html file
=======
    static defaultEncodeAs = [taglib:'none']

    def utilsService

    /** Renders a html file from gp-server
>>>>>>> master
     * used for rendering pipeline report results
     */
    def render = {attrs ->
        AnalysisServer analysisServer = AnalysisServer.get(attrs?.analysisServerId)
        if(attrs.href && attrs.href!='' && analysisServer) {
            def fileUrl = new URL(attrs.href)
            def connection = fileUrl.openConnection()
            connection.setRequestProperty ("Authorization", utilsService.authHeader(analysisServer.userName, analysisServer.userPw))
<<<<<<< HEAD
            def dataStream = connection.inputStream
            out << dataStream
        }
        else {
            out << 'Error: no report file found!'
        }
    }
=======
import org.jsoup.Jsoup
import org.jsoup.nodes.Document
import org.jsoup.nodes.Element
import org.jsoup.nodes.Node
import org.jsoup.nodes.TextNode


class FgUtilsTagLib {

    static namespace = 'fg'
    static defaultEncodeAs = [taglib:'none']

    def utilsService
>>>>>>> master

    /** Renders a html file from gp-server
     * used for rendering pipeline report results
     */
    def render = {attrs ->
        AnalysisServer analysisServer = AnalysisServer.get(attrs?.analysisServerId)
        if(attrs.href && attrs.href!='' && analysisServer) {
            def fileUrl = new URL(attrs.href)
            def connection = fileUrl.openConnection()
            connection.setRequestProperty ("Authorization", utilsService.authHeader(analysisServer.userName, analysisServer.userPw))
=======
>>>>>>> master
            String outPut
            def dataStream = connection.inputStream
            outPut = dataStream.text
//            Document doc = Jsoup.connect(attrs.href).header("Authorization", utilsService.authHeader(analysisServer.userName, analysisServer.userPw)).get()
//            Element element = doc.getElementById("notebook")
//            outPut = element?.text()
//            outPut = doc?.text()
            out << outPut
        }
        else {
            out << 'Error: no report file found!'
        }
    }


    /*
    def render2 = {attrs ->
        AnalysisServer analysisServer = AnalysisServer.get(attrs?.analysisServerId)
        if(attrs.href && attrs.href!='' && analysisServer) {
            String outPut ="""
                            <script src="https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.1/MathJax.js?config=TeX-AMS_HTML"></script>
                            <script type="text/x-mathjax-config;executed=true">
                                MathJax.Hub.Config({
                                    tex2jax: {
                                        inlineMath: [ ['\$','\$'], ["\\\\(","\\\\)"] ],
                                        displayMath: [ ['\$\$','\$\$'], ["\\\\[","\\\\]"] ],
                                        processEscapes: true,
                                        processEnvironments: true
                                    },
                                    // Center justify equations in code and markdown cells. Elsewhere
                                    // we use CSS to left justify single line equations in code cells.
                                    displayAlign: 'center',
                                    "HTML-CSS": {
                                        styles: {'.MathJax_Display': {"margin": 0}},
                                        linebreaks: { automatic: true }
                                    }
                                });
                            </script>
                            """
            Document document = Jsoup.connect(attrs.href).header("Authorization", utilsService.authHeader(analysisServer.userName, analysisServer.userPw)).get()
            Element element = document.getElementById("notebook")
            outPut += element?.text()
            out << outPut
        }
        else {
            out << 'Error: no report file found!'
        }
    }
    */

    def renderHtmlFile = {attrs ->
        def htmlFile = grailsApplication.mainContext.getResource("classpath:resources/$attrs.fileName").file
        out << htmlFile.text
    }

}
