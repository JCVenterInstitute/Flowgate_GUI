package flowgate

class ExpFileMetadata {

    static belongsTo = [ExpFile]
    ExpFile expFile
    String mdKey
    String mdVal
    Integer dispOrder

    static constraints = {
        mdKey nullable: false
        mdVal nullable: true
        dispOrder nullable: true
    }
}
