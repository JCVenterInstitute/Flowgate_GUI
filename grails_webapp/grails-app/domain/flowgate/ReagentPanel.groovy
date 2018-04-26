package flowgate

class ReagentPanel {

//    static belongsTo = [ExpFile]

    static hasMany = [reagents: Reagent]

    String reagentPanelName

    static constraints = {
        reagentPanelName blank: true
    }

    static mapping ={
        reagentPanelName sqlType: 'varchar(512)'
        reagents         nullable : true
    }
}
