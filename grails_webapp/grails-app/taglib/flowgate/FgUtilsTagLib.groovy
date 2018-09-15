package flowgate

import grails.converters.JSON
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
    def renderRes = {attrs ->
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
    def renderRes2 = {attrs ->
        AnalysisServer analysisServer = AnalysisServer.get(attrs?.analysisServerId)
        if(attrs.href && attrs.href!='' && analysisServer) {
            Document document = Jsoup.connect(attrs.href).header("Authorization", utilsService.authHeader(analysisServer.userName, analysisServer.userPw)).get()
            Element element = document.getElementById("notebook")
            /* out << """
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
            */
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


    /**
     * Renders the template that allows to dynamically add and remove identical HTML blocks
     * (hereinafter "items").
     *
     * @attr itemId          REQUIRED The prefix for the item id (every item id consists of
     *                                the prefix and the index number)
     * @attr template        OPTIONAL The name of a GSP template that contains HTML code for
     *                                every item (if missing, the tag's body will be used)
     * @attr model           OPTIONAL The model passed to the GSP template
     * @attr addBtnId        OPTIONAL The id of the 'add' button, the page must contain
     *                                an element with this id to provide custom 'add' button;
     *                                if isn't specified, the default 'add' button
     *                                will be rendered
     * @attr removeBtnLabel  OPTIONAL The label of the 'remove' button that is rendered
     *                                for each item (defaults to 'Remove')
     * @attr min             OPTIONAL The minimum number of items (the number of items that are
     *                                rendered by default)
     * @attr max             OPTIONAL The maximum number of items that can be added to the page
     * @attr limitReachedMsg OPTIONAL The message displayed when the limit is reached
     * @attr onComplete      OPTIONAL The name of a JS function that will be executed right
     *                                after a new item is added (must accept
     *                                the item index number)
     */
    def dynamicBlock = { attrs, body ->
        println "in dynamicsBlock"
        // checks if the itemId attribute is passed to the tag
        def id = attrs.itemId
        if (!id) throwTagError("[id] attribute must be specified")

        // validates the min and max attributes
        def min
        def max
        try {
//            min = attrs.mdVals.size() ?: 1
            min = 1
            max = attrs.max ? attrs.max as int : null
        } catch (NumberFormatException nfe) {
            throwTagError("[min] and [max] attributes must be integer numbers")
        }
        if (min && max && max < min) {
            throwTagError("[min] attribute must be less than [max]")
        }

        // prepares template for new items
        def elem = attrs.template ? render(template: attrs.template, model: attrs.model) : body()
        elem = elem.replaceAll('\n', '') // makes the template single-lined in order to pass it as a parameter to JS function that adds new items
        elem = elem.encodeAsJavaScript() // makes the template able to pass into a JS function

        // renders GSP template with auxiliary HTML and JS code
        String output = render(template: "/templates/dynamicBlock/add", model: [
            id: id,
            elem: elem,
            addBtnId: attrs.addBtnId,
            removeBtnLabel: attrs.removeBtnLabel,
            min: min ?: 0,
            max: max ?: 0,
            limitReachedMsg: attrs.limitReachedMsg,
            onComplete: attrs.onComplete,
//            mdVals: attrs.mdVals.collect{"\"$it\""}
            mdVals: attrs.mdVals
//            mdVals: attrs.mdVals as
        ])
//        println "output ${output}"
        /*out << render(template: "/templates/dynamicBlock/add", model: [
            id: id,
            elem: elem,
            addBtnId: attrs.addBtnId,
            removeBtnLabel: attrs.removeBtnLabel,
            min: min ?: 0,
            max: max ?: 0,
            limitReachedMsg: attrs.limitReachedMsg,
            onComplete: attrs.onComplete
        ])*/
        out << output
    }


}
