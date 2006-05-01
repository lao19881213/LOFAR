/*
 * EmptyDataSetException.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.java.gui.plotter.exceptions;

import nl.astron.lofar.java.gui.plotter.PlotConstants;

/**
 * @version $Id$
 * @created April 18, 2006, 11:02 AM
 * @author pompert
 */
public class EmptyDataSetException extends PlotterException{
    
    /** Creates a new instance of EmptyDataSetException */
    public EmptyDataSetException() {
        super();
    }
    public String getMessage(){
        return super.getMessage() + PlotConstants.EXCEPTION_EMPTY_DATASET;
    }
    
    
}
