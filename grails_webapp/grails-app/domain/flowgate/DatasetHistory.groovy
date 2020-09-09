package flowgate

class DatasetHistory implements Serializable {

    private static final long serialVersionUID = 1

    static belongsTo = [Experiment]

    Experiment experiment
    Long datasetId
    Long version
    String name
    String description

    static constraints = {
        description nullable: true
    }

}
