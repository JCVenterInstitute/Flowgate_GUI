package flowgate

class DatasetHistory implements Serializable {

    private static final long serialVersionUID = 1

    static belongsTo = [Experiment]

    Experiment experiment
    Long did
    Long version
    String name
    String description

    static constraints = {
        description nullable: true
    }

//    static mapping = {
//        version true
//    }
}
