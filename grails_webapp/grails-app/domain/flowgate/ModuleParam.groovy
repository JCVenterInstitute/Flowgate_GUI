package flowgate

class ModuleParam {

    static belongsTo = [Module]
    Module module
    Boolean pBasic
    String pType
    String pKey
    String defaultVal

    static constraints = {
        defaultVal nullable: true
        pType nullable: true // , inList: ["file","url","val","none", null]
    }
}
