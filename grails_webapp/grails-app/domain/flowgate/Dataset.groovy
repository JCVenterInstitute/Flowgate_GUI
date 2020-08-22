package flowgate

class Dataset implements Serializable {

//    private static final long serialVersionUID = 1

    static belongsTo = Experiment
    static hasMany = [expFiles: ExpFile]

    Experiment experiment
    String name
    String description

    static constraints = {
        description nullable: true
    }

}
