package flowgate

class AnalysisServer {

    String name
    String url
    String userName
    String userPw

    static constraints = {
        userPw nullable: true, blank: true
    }
}
