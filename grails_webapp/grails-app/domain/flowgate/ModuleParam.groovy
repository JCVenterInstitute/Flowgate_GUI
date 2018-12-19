package flowgate

class ModuleParam {

    static belongsTo = [Module]
    Module module
    Boolean pBasic
    Integer pOrder
    String pLabel
    String pType
    String pKey
    String defaultVal
    String descr
    String exampleFile

    static constraints = {
        defaultVal nullable: true
        pBasic: true
        pType nullable: true // , inList: ["ds","dir","file","url","val","none", null]
        pLabel nullable: true
        pOrder nullable: true
        descr nullable: true
        exampleFile nullable: true
    }
}
