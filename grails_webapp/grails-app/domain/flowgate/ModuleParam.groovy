package flowgate

class ModuleParam {

    static belongsTo = [Module]
    Module module
    Boolean pBasic
    String pLabel
    String pType
    String pKey
    String defaultVal
    String descr

    static constraints = {
        defaultVal nullable: true
        pType nullable: true // , inList: ["ds","dir","file","url","val","none", null]
        pLabel nullable: true
        descr nullable: true
    }
}
