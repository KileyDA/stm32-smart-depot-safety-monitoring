#Parse and validate incoming serial messages
# keeping communication logic separate from future website
def parse_status(message):
    parts = message.split(",")
    
    #Expected format:STATE,adc,motion,alert,acknowledged,pwm
    if len(parts) != 6 or parts[0] != "STATE":
        return None
    
    try:
        adc,motion,alert,acknowledged,pwm = map(int, parts[1:])
    except ValueError:
        return None
    
    #Reject malformed or out-of-range readings
    if not (0 <= adc <= 4095 and 0 <= pwm <= 100):
        return None
    
    if any(value not in (0, 1) for value in (motion, alert, acknowledged)):
        return None
    
    return {
        "adc_value": adc,
        "motion_detected": bool(motion),
        "alert_active": bool(alert),
        "alert_acknowledged": bool(acknowledged),
        "brightness": pwm,
    }
