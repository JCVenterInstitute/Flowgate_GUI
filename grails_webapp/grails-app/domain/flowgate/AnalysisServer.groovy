package flowgate

class AnalysisServer {

    User user
    String name
    String url
    String userName
    String userPw
    Integer platform

    static constraints = {
        userPw type: EncryptedString, nullable: true, blank: true
//        userPw nullable: true, blank: true
        user nullable: true, blank: true
    }

    def isImmportGalaxyServer() {
        return platform == 2
    }

    def isGenePatternServer() {
        return platform == 1
    }
}
