package flowgate

class ExperimentMetadata {

    static belongsTo = [Experiment, ExpFile]
    static hasMany = [expFiles: ExpFile, mdVals: ExperimentMetadataValue]
//    static hasOne = [mdCategory: ExperimentMetadataCategory]
    Experiment experiment
//    String mdCategory
    ExperimentMetadataCategory mdCategory
    String mdKey
    Integer dispOrder
    Boolean dispOnFilter = true
    Boolean visible = true

    static constraints = {
        mdCategory nullable: true, blank: true //, inList: ['Demographics','Visit','Stimulation','Reagents']
        expFiles nullable: true, blank: true
        mdVals nullable: true, blank: true

    }
}
