package flowgate

class Experiment implements Serializable  {

    static belongsTo = [Project]
    static hasMany = [expFiles: ExpFile, analyses: Analysis, expMetadatas: ExperimentMetadata ]
    Project project
    String title
    String description
    List analyses
    String experimentHypothesis
    String experimentMeta
    Boolean isActive = true

    static constraints = {
        title unique: ['project']
        experimentMeta blank: true, nullable: true
        experimentHypothesis blank: true, nullable: true
    }

    static mapping = {
        description sqlType: 'varchar(1024)'
        experimentHypothesis sqlType:  'varchar(2048)'
        experimentMeta sqlType: 'varchar(512)'
    }
}
