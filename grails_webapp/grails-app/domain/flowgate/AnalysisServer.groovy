package flowgate

class AnalysisServer {

    User user
    String name
    String url
    String userName
    String userPw

    static constraints = {
        userPw type: EncryptedString, nullable: true, blank: true
//        userPw nullable: true, blank: true
        user nullable: true, blank: true
    }
}
