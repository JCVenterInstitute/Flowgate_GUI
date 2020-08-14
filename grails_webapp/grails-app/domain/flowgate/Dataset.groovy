package flowgate

class Dataset {

    static belongsTo = [experiment: Experiment, analysis: Analysis]
    static hasMany = [expFiles: ExpFile]
//
    static mapping = {
        analysis joinTable: [name: 'analysis_dataset', column: "dataset_id"]
    }
//
    Experiment experiment
    String name
    String description

    static constraints = {
        description nullable: true
    }


}
