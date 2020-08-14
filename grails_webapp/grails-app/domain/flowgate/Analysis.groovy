package flowgate

import grails.databinding.BindingFormat
import org.apache.commons.lang3.math.NumberUtils

class Analysis {

  static belongsTo = [experiment: Experiment]
  static hasMany = [datasets: Dataset]

  static mapping = {
    analysisName    sqlType: 'varchar(512)'
    analysisDescription  sqlType: 'varchar(1024)'
    datasets joinTable: [name: 'analysis_dataset', column: "analysis_id"]
  }

  Module module
  String analysisName
  String analysisDescription
  User user
  @BindingFormat('yyyy-MM-dd hh:mm:ss')
  Date timestamp
  @BindingFormat('yyyy-MM-dd hh:mm:ss')
  Date dateCreated
  @BindingFormat('yyyy-MM-dd hh:mm:ss')
  Date dateCompleted
  Integer analysisStatus
  String jobNumber
  // TODO check different types! currently using the result file path/name to render in the modal
  // TODO current default resultReportFileName = Reports/AutoReport.html; remove after testing
  String renderResult = 'Reports/AutoReport.html'

  static constraints = {
    analysisName blank: false
    analysisDescription nullable: true, blank: true, blankable: true
    jobNumber nullable: true, blank: true, blankable: true
    renderResult blank: true, nullable: true
    dateCompleted blank: true, nullable: true
  }

  def isFailedOnSubmit() {
    return NumberUtils.isNumber(jobNumber) && -1 == Integer.parseInt(jobNumber)
  }

  /**
   * Defines the different Analysis Status.
   */
  public enum Status {

    /**
     * Analysis failed
     */
    DELETED(-2),

    /**
     * Analysis failed
     */
    FAILED(-1),

    /**
     * Analysis processing
     */
    PROCESSING(2),

    /**
     * Analysis completed
     */
    FINISHED(3),

    /**
     * Analysis completed but report file is missing
     */
    REPORT_FILE_MISSING(4);

    private final int statusValue;

    private Status(final int statusValue) {
      this.statusValue = statusValue;
    }

    public int value() {
      return statusValue;
    }
  }
}
