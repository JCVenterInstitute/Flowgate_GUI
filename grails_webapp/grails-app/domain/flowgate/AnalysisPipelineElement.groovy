package flowgate

class AnalysisPipelineElement {

    static belongsTo = [AnalysisPipeline]
    Module module
    Integer position

    static constraints = {
    }
}
