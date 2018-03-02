package flowgate

class AnalysisPipeline {

    static belongsTo = [experiment: Experiment]
    static hasMany = [elements: AnalysisPipelineElement]
    String name
    static constraints = {
    }
}
