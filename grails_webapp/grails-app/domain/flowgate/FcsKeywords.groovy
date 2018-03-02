package flowgate

class FcsKeywords {

    String keyName
    String comment
    Boolean active
    Boolean safe
    Boolean flowRepo

    static constraints = {
        comment nullable: true, blank: true
    }
}
