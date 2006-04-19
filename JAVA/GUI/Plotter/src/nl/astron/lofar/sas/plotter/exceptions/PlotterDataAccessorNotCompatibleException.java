/*
 * PlotterDataAccessorNotCompatibleException.java
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
 * @created April 19, 2006, 11:02 AM
 * @author pompert
 */
public class PlotterDataAccessorNotCompatibleException extends PlotterException{
    
    /** Creates a new instance of PlotterDataAccessorNotCompatibleException */
    public PlotterDataAccessorNotCompatibleException() {
        super();
    }
    public String getMessage(){
        return PlotConstants.EXCEPTION_DATA_ACCESS_NOT_COMPATIBLE;
    }
    
    
}
