package flowgate

class Reagents {

    static belongsTo = [ReagentPanel]
    ReagentPanel reagentPanel
    String channel
    String fluorochrome
    String marker
    String transformName
    String inputMin
    String inputMax
    String outputMin
    String outputMax

    static constraints = {
    }

    static mapping = {
        channel         sqlType: 'varchar(512)'
        fluorochrome         sqlType: 'varchar(512)'
        marker      sqlType: 'varchar(512)'
        transformName   sqlType: 'varchar(512)'
        inputMin        sqlType: 'varchar(512)'
        inputMax        sqlType: 'varchar(512)'
        outputMin       sqlType: 'varchar(512)'
        outputMax       sqlType: 'varchar(512)'
    }
}
