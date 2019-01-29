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
  // TODO check different types! currently using the result file path/name to render in the modal
  // TODO current default resultReportFileName = Reports/AutoReport.html; remove after testing
  String renderResult = 'Reports/AutoReport.html'

  static constraints = {
    analysisName blank: false
    analysisDescription nullable: true, blank: true, blankable: true
    jobNumber nullable: true, blank: true, blankable: true
    renderResult blank: true, nullable: true
  }

  static mapping = {
    analysisName    sqlType: 'varchar(512)'
    analysisDescription  sqlType: 'varchar(1024)'
  }
}
