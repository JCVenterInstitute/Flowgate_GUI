package flowgate.transform

class RectangleGate extends Gate {

    def markerName
    def markerLabel
    def maxByDimension
    def minByDimension
    def transformationRef

    @Override
    public String toString() {
        return "RectangleGate{" +
                "markerName=" + markerName +
                ", markerLabel=" + markerLabel +
                ", maxByDimension=" + maxByDimension +
                ", minByDimension=" + minByDimension +
                ", transformationRef=" + transformationRef +
                '}';
    }
}
