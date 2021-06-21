package flowgate

class TransformationParameter {

    static hasMany = [reagents: Reagent]

    String transformName
    String transformType
    String parameterValues
    Boolean isPredefined

    static constraints = {
    }
}
