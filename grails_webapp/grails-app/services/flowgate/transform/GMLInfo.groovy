package flowgate.transform

import org.xml.sax.SAXException

import javax.xml.parsers.ParserConfigurationException

class GMLInfo {

    def file;

    def transforms = [:]
    def gates = [:]

    enum TransformType {
        NONE,
        LINEAR,
        LOG,
        LOGICLE
    }

    public GMLInfo(file) {
        this.file = file;

        readFile();
    }

    private def getTransformTypeName(def transformationType) {
        switch (transformationType) {
            case LINEAR:
                return "linear";
            case LOG:
                return "log";
            case LOGICLE:
                return "logicle";
            default:
                return "none";
        }
    }

    def findTransformTypeByName(def name) {
        if (name == "linear")
            return LINEAR;
        if (name == "log")
            return LOG;
        if (name == "logicle")
            return LOGICLE;

        return NONE;
    }

    private void readFile() {
        try {
            def gml = new XmlParser().parse(this.file);

            def index = 1
            gml.value().each { it ->
                def elementName = it.name().localPart

                if (elementName.equals("transformation")) {
                    def namespaceURI = it.name().namespaceURI

                    def transformationAttribute = it.attributes().find {
                        it.key.namespaceURI = namespaceURI
                    }

                    def transformationDetails = it.value()[0]
                    def transformationType = transformationDetails.name().localPart
                    def transformationId = transformationAttribute.value
                    def values = transformationDetails.attributes().collectEntries { [it.key.localPart, it.value as double] }

                    println "Transform Id: $transformationId"
                    switch (transformationType) {
                        case "logicle":
                            def logicleTransform = new LogicleTransform(values["T"], values["A"], values["M"], values["W"])
                            logicleTransform.id = transformationId
                            logicleTransform.transformType = transformationType

                            transforms[transformationId] = logicleTransform

                            println logicleTransform
                            break;
                        case "flin":
                            def linearTransform = new ParameterizedLinearTransform(values["T"], values["A"])
                            linearTransform.id = transformationId
                            linearTransform.transformType = transformationType

                            transforms[transformationId] = linearTransform

                            println linearTransform
                            break;
                        case "flog":
                            def logTransform = new ParameterizedLogarithmicTransform(values["T"], values["M"])
                            logTransform.id = transformationId
                            logTransform.transformType = transformationType

                            transforms[transformationId] = logTransform

                            println logTransform
                            break;
                    }

                } else if (elementName.equals("RectangleGate")) {
                    def namespaceURI = it.name().namespaceURI
                    def gateId = it.attributes().find {
                        it.key.namespaceURI = namespaceURI && it.key.localPart.equals("id")
                    }
                    def gateParentId = it.attributes().find {
                        it.key.namespaceURI = namespaceURI && it.key.localPart.equals("parent_id")
                    }
                    def gateObj = new Gate()
                    gateObj.order = index++
                    gateObj.id = gateId.value
                    gateObj.type = "RectangleGate"

                    if (gateParentId) {
                        gateObj.parentId = gateParentId.value
                    }

                    def gateMarkerNameArr;
                    def gateIndex = 0;
                    it.value().each {
                        node ->
                            if (node.name().localPart.equals("dimension")) {
                                def rectangleGate = new RectangleGate()

                                if (gateMarkerNameArr) {
                                    rectangleGate.markerLabel = gateMarkerNameArr[gateIndex++]
                                }

                                def gateNode = node.value().find {
                                    it.name().localPart.equals("fcs-dimension")
                                }.attributes().find {
                                    it.key.localPart.equals("name")
                                }

                                rectangleGate.markerName = gateNode.value

                                node.attributes().each {
                                    attr ->
                                        if (attr.key.localPart.equals("min")) {
                                            rectangleGate.minByDimension = attr.value as double
                                        } else if (attr.key.localPart.equals("max")) {
                                            rectangleGate.maxByDimension = attr.value as double
                                        } else if (attr.key.localPart.equals("transformation-ref")) {
                                            rectangleGate.transformationRef = attr.value
                                        }
                                }

                                println rectangleGate
                                gateObj.children.add(rectangleGate)
                            } else if (node.name().localPart.equals("custom_info")) {
                                def flowgateInfo = node.value().find {
                                    it.name().equals("flowgate")
                                }

                                if (flowgateInfo.value()) {
                                    def gateMarkerNames = flowgateInfo.value().find {
                                        it.name().equals("name")
                                    }

                                    if (gateMarkerNames && gateMarkerNames.value()) {
                                        gateMarkerNameArr = gateMarkerNames.value()[0].split(" vs ")
                                        println gateMarkerNameArr
                                    }
                                }
                            }
                    }

                    gates[gateId.value] = gateObj
                } else if (elementName.equals("PolygonGate")) {
                    def namespaceURI = it.name().namespaceURI
                    def gateId = it.attributes().find {
                        it.key.namespaceURI = namespaceURI && it.key.localPart.equals("id")
                    }
                    def gateParentId = it.attributes().find {
                        it.key.namespaceURI = namespaceURI && it.key.localPart.equals("parent_id")
                    }
                    def gateObj = new Gate()
                    gateObj.order = index++
                    gateObj.id = gateId.value
                    gateObj.type = "PolygonGate"

                    if (gateParentId) {
                        gateObj.parentId = gateParentId.value
                    }

                    def polygonGate = new PolygonGate()
                    def gateMarkerNameArr;
                    def gateIndex = 0;
                    it.value().each {
                        node ->
                            if (node.name().localPart.equals("dimension")) {
                                polygonGate.markerLabels.push(gateMarkerNameArr ? gateMarkerNameArr[gateIndex++] : "")

                                def gateNode = node.value().find {
                                    it.name().localPart.equals("fcs-dimension")
                                }.attributes().find {
                                    it.key.localPart.equals("name")
                                }

                                polygonGate.markerNames.push(gateNode.value)

                                node.attributes().each {
                                    attr ->
                                        if (attr.key.localPart.equals("transformation-ref")) {
                                            polygonGate.transformationRefs.push(attr.value)
                                        }
                                }
                            } else if (node.name().localPart.equals("vertex")) {
                                def values = node.value().findAll {
                                    it.name().localPart.equals("coordinate")
                                }

                                def xCoordinate = values[0].attributes().find {
                                    it.key.localPart.equals("value")
                                }

                                def yCoordinate = values[1].attributes().find {
                                    it.key.localPart.equals("value")
                                }

                                polygonGate.xCoordinates.push(xCoordinate.value as double)
                                polygonGate.yCoordinates.push(yCoordinate.value as double)
                            } else if (node.name().localPart.equals("custom_info")) {
                                def flowgateInfo = node.value().find {
                                    it.name().equals("flowgate")
                                }

                                if (flowgateInfo.value()) {
                                    def gateMarkerNames = flowgateInfo.value().find {
                                        it.name().equals("name")
                                    }

                                    if (gateMarkerNames && gateMarkerNames.value()) {
                                        gateMarkerNameArr = gateMarkerNames.value()[0].split(" vs ")
                                        println gateMarkerNameArr
                                    }
                                }
                            }
                    }

                    gateObj.children.add(polygonGate)

                    gates[gateId.value] = gateObj

                }
            }
        } catch (IOException | SAXException | ParserConfigurationException e) {
            e.printStackTrace();
        }
    }
}
