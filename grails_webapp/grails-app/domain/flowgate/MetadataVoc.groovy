package flowgate

class MetadataVoc {


    String mdKey
    String mdVal
    String category
    Integer dispOrder
    String status

    static constraints = {
        mdVal nullable: true, blank: true
    }
}
