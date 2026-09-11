public class AuditService implements IAuditable {
    public String lastLog;

    public void logMessage(String msg) {
        this.lastLog = '[AUDIT] ' + msg;
    }

    public String getAuditStatus() {
        return this.lastLog;
    }
}
