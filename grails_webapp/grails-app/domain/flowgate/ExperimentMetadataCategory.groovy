package flowgate

class ExperimentMetadataCategory {

//    static belongsTo = [ExperimentMetadata]

    Experiment experiment
    String mdCategory
    Integer dispOrder = 1
    Boolean dispOnFilter = true
    Boolean visible = true
    Boolean isDefault = false

    static constraints = {
        experiment nullable: false
        mdCategory nullable: false, blank: true
        dispOnFilter nullable: true, blank: true
        isDefault nullable: true, blank: true
    }

//    String toString() {
//        this.mdCategory
//    }
}
