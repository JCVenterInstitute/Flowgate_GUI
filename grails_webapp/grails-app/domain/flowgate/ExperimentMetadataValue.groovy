package flowgate

class ExperimentMetadataValue {

    static belongsTo = [expMetaData: ExperimentMetadata]
    String mdValue
    Integer dispOrder
    String mdType

    static constraints = {
        expMetaData nullable: true
    }
}
