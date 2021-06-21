package flowgate

class ReagentPanel {

    static hasMany = [reagents: Reagent]

    User user
    String reagentPanelName

    static constraints = {
        user nullable: false
        reagentPanelName blank: true
    }

    static mapping ={
        reagentPanelName sqlType: 'varchar(512)'
        reagents         nullable : true, sort: 'dispOrder', order: 'asc'
    }
}
