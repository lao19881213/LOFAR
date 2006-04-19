/*
 * NotImplementedException.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter.exceptions;

import nl.astron.lofar.sas.plotter.PlotConstants;

/**
 * @version $Id$
 * @created April 19, 2006, 11:00 AM
 * @author pompert
 */
public class NotImplementedException extends PlotterException{
    
    private String message;
    /** Creates a new instance of NotImplementedException */
    public NotImplementedException(String message) {
        super();
        this.message = message;
    }
    public String getMessage(){
        return PlotConstants.EXCEPTION_NOTIMPLEMENTED + message;
    }
    
    
}
