package flowgate

import grails.databinding.BindingFormat

class Analysis {

    static belongsTo = [experiment: Experiment]
    static hasMany = [datasets: Dataset]
    Module module
    String analysisName
    String analysisDescription
    User user
    @BindingFormat('yyyy-MM-dd hh:mm:ss')
    Date timestamp
    @BindingFormat('yyyy-MM-dd hh:mm:ss')
    Date dateCreated
    Integer analysisStatus
    Integer jobNumber

    static constraints = {
        analysisDescription nullable: true, blank: true, blankable: true
        jobNumber nullable: true, blank: true, blankable: true
    }

    static mapping = {
        analysisName    sqlType: 'varchar(512)'
        analysisDescription  sqlType: 'varchar(1024)'
    }
}
