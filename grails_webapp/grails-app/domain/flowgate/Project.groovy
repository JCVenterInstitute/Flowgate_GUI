package flowgate

class Project implements Serializable  {

    static hasMany = [experiments: Experiment]
    String title
    String description
    Boolean isActive = true

    static constraints = {  }

    static mapping = {
        description sqlType: 'varchar(2048)'
    }
}
