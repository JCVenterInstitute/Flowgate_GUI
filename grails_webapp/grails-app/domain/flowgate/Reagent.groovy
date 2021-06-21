package flowgate

class Reagent {

    static belongsTo = [ReagentPanel]
    ReagentPanel reagentPanel
    TransformationParameter transformationParameter
    String channel
    String alias
    String fluorochrome
    String marker
    String transformName
    String inputMin
    String inputMax
    String outputMin
    String outputMax
    Boolean isVisible = true
    Integer dispOrder = 0

    String toString() {
        if(isVisible){
            channel + "${(marker || fluorochrome) ? '-ZZ'+ marker ? marker : fluorochrome ? '/'+fluorochrome : 'ZZ '  : '' }"   ///?: '' + fluorochrome ? '/'+fluorochrome : '' : '] '
        } else {
            ''
        }
    }

    static constraints = {
        channel         nullable: true, blank: true, sqlType: 'varchar(512)'
        alias         nullable: true, blank: true, sqlType: 'varchar(512)'
        fluorochrome    nullable: true, blank: true, sqlType: 'varchar(512)'
        marker          nullable: true, blank: true, sqlType: 'varchar(512)'
        transformName   nullable: true, blank: true, sqlType: 'varchar(512)'
        inputMin        nullable: true, blank: true, sqlType: 'varchar(512)'
        inputMax        nullable: true, blank: true, sqlType: 'varchar(512)'
        outputMin       nullable: true, blank: true, sqlType: 'varchar(512)'
        outputMax       nullable: true, blank: true, sqlType: 'varchar(512)'
        transformationParameter nullable: true, blank: true
    }

    static mapping = {
        channel         nullable: true, blank: true, sqlType: 'varchar(512)'
        alias         nullable: true, blank: true, sqlType: 'varchar(512)'
        fluorochrome    nullable: true, blank: true, sqlType: 'varchar(512)'
        marker          nullable: true, blank: true, sqlType: 'varchar(512)'
        transformName   nullable: true, blank: true, sqlType: 'varchar(512)'
        inputMin        nullable: true, blank: true, sqlType: 'varchar(512)'
        inputMax        nullable: true, blank: true, sqlType: 'varchar(512)'
        outputMin       nullable: true, blank: true, sqlType: 'varchar(512)'
        outputMax       nullable: true, blank: true, sqlType: 'varchar(512)'
        transformationParameter nullable: true, blank: true
    }
}
