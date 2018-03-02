package flowgate

class Module {

    static hasMany=[moduleParams: ModuleParam]
    String title
    String name
    AnalysisServer server

    static constraints = {
        moduleParams nullable: true
    }
}
