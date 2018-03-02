package flowgate

class ExpFile {

    static belongsTo = [Experiment]
    static hasMany = [metaDatas: ExpFileMetadata, expMetaData: ExperimentMetadata]
    static hasOne = [ReagentPanel]
    Experiment experiment
    ReagentPanel reagentPanel
    String title
    String fileName
    String filePath
    String chkSum
    User createdBy
    Date dateCreated
    Boolean isActive = true

    static constraints = {
        reagentPanel nullable: true
    }
}
