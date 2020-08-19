package flowgate

class Dataset implements Serializable {

    private static final long serialVersionUID = 1

//    static belongsTo = [experiment: Experiment, analysis: Analysis]
    static belongsTo = [experiment: Experiment]
//    static hasMany = [expFiles: ExpFile, analysis: Analysis]
    static hasMany = [expFiles: ExpFile]
//
//    static mapping = {
//        analysis joinTable: [name: 'analysis_dataset', column: "dataset_id"]
//    }

    Experiment experiment
    String name
    String description

    static constraints = {
        description nullable: true
    }


}
