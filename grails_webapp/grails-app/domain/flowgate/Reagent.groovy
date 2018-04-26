package flowgate

class Reagent {

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
    Integer dispOrder = 0

    static constraints = {
    }

    static mapping = {
        channel         nullable: true, blank: true, sqlType: 'varchar(512)'
        fluorochrome    nullable: true, blank: true, sqlType: 'varchar(512)'
        marker          nullable: true, blank: true, sqlType: 'varchar(512)'
        transformName   nullable: true, blank: true, sqlType: 'varchar(512)'
        inputMin        nullable: true, blank: true, sqlType: 'varchar(512)'
        inputMax        nullable: true, blank: true, sqlType: 'varchar(512)'
        outputMin       nullable: true, blank: true, sqlType: 'varchar(512)'
        outputMax       nullable: true, blank: true, sqlType: 'varchar(512)'
    }
}
