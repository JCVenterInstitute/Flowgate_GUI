package flowgate

class Module {

    static hasMany=[moduleParams: ModuleParam]
    String label
    String title
    String name
    String descript
    AnalysisServer server

    static constraints = {
        moduleParams nullable: true
        descript nullable: true
        label nullable: true
    }

    static mapping = {
        descript sqlType: 'varchar(4096)'
    }
}
