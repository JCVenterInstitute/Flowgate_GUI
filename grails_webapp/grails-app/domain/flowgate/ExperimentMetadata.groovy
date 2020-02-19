package flowgate

class ExperimentMetadata {

    static belongsTo = [Experiment, ExpFile]
    static hasMany = [expFiles: ExpFile, mdVals: ExperimentMetadataValue]

    Experiment experiment
    ExperimentMetadataCategory mdCategory
    String mdKey
    Integer dispOrder = 1
    Boolean dispOnFilter = true
    Boolean visible = true
    Boolean isMiFlow = false

    static constraints = {
        mdCategory nullable: true, blank: true //, inList: ['Demographics','Visit','Stimulation','Reagents']
        expFiles nullable: true, blank: true
        mdVals nullable: true, blank: true
        isMiFlow nullable: true, blank: true

    }
}
