package flowgate

class Dataset {

    static belongsTo = [Experiment]
    static hasMany = [expFiles: ExpFile]
    Experiment experiment
    String name
    String description
    Boolean isActive = true

    static constraints = {
        description nullable: true
    }
}
