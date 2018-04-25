package flowgate

class AnalysisServer {

    User user
    String name
    String url
    String userName
    String userPw

    static constraints = {
        userPw nullable: true, blank: true
    }
}
