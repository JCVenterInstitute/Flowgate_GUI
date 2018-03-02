package flowgate

class Reagents {

    static belongsTo = [ReagentPanel]
    ReagentPanel reagentPanel
    String markerName
    String dyeName
    String transformName
    String inputMin
    String inputMax
    String outputMin
    String outputMax

    static constraints = {
    }

    static mapping = {
        markerName      sqlType: 'varchar(512)'
        dyeName         sqlType: 'varchar(512)'
        transformName   sqlType: 'varchar(512)'
        inputMin        sqlType: 'varchar(512)'
        inputMax        sqlType: 'varchar(512)'
        outputMin       sqlType: 'varchar(512)'
        outputMax       sqlType: 'varchar(512)'
    }
}
