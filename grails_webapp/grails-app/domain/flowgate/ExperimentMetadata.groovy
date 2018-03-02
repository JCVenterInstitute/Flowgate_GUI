package flowgate

class ExperimentMetadata {

    static belongsTo = [Experiment, ExpFile]
    static hasMany = [expFiles: ExpFile, mdVals: ExperimentMetadataValue]
    Experiment experiment
    String mdCategory
    String mdKey
    Integer dispOrder

    static constraints = {
        mdCategory nullable: true, blank: true
        expFiles nullable: true, blank: true
        mdVals nullable: true, blank: true

    }
}
