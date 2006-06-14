/*
 * StatusPanel.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

package nl.astron.lofar.java.gui.mac.jrsp.status;

import javax.swing.JPanel;
import nl.astron.lofar.java.mac.jrsp.BoardStatus;
import nl.astron.lofar.java.mac.jrsp.RSPMask;
import nl.astron.lofar.java.gui.mac.jrsp.*;
import org.apache.log4j.Logger;

/**
 * The StatusPanel displays the Status of the RSPboard (BoardStatus) that is 
 * passed to this class through the setFields() method. It delegates all the 
 * "displaying" to other panels.
 * 
 * @author balken
 * @see ADOStatusPanel
 * @see DIAGStatusPanel
 * @see ETHStatusPanel
 * @see MEPStatusPanel
 * @see RCUStatusPanel
 * @see RSPStatusPanel
 * @see RSUStatusPanel
 * @see SyncStatusPanel
 */
public class StatusPanel extends JPanel implements ITabPanel
{
    /** The main panel. */
    private MainPanel mainPanel;
    
    /** Logger */
    private Logger itsLogger;
    
    /** 
     * Creates new form StatusPanel.
     */
    public StatusPanel() 
    {
        itsLogger = Logger.getLogger(getClass());
        itsLogger.info("Constructor");
        
        initComponents();
        
        blp0SyncStatusPanel.setTitle("BLP0 Sync");
        blp1SyncStatusPanel.setTitle("BLP1 Sync");
        blp2SyncStatusPanel.setTitle("BLP2 Sync");
        blp3SyncStatusPanel.setTitle("BLP3 Sync");
        blp0RcuStatusPanel.setTitle("BLP0 RCU");
        blp1RcuStatusPanel.setTitle("BLP1 RCU");
        blp2RcuStatusPanel.setTitle("BLP2 RCU");
        blp3RcuStatusPanel.setTitle("BLP3 RCU");
        blp0AdoStatusPanel.setTitle("BLP0 ADO");
        blp1AdoStatusPanel.setTitle("BLP1 ADO");
        blp2AdoStatusPanel.setTitle("BLP2 ADO");
        blp3AdoStatusPanel.setTitle("BLP3 ADO");
    }
    
    /**
     * Used to initialize the ITabPanel and give it a refrence to the main panel.
     * 
     * @param mainPanel   The MainPanel.
     */
    public void init(MainPanel mainPanel)
    {        
        this.mainPanel = mainPanel;
    }
    
    /**
     * Method that can be called by the main panel to update this panel.
     */
    public void update(int updateType)
    {
        if (!mainPanel.getBoard().isConnected()) {
            //setFields(null); // clears all the inputfields.
            return;            
        }
        
        RSPMask mask = new RSPMask(mainPanel.getSelectedBoardIndex());
        setFields(mainPanel.getBoard().getStatus(mask)[0]);
    }
    
    /**
     * Method that can be called to disable or enable the board.
     * @param   b       Boolean value used to determine to enable (true) or
     *                  disable (false).
     */
    public void enablePanel(boolean b) {}    
    
    /**
     * Sets the fields on the panel to the right values.
     */
    public void setFields(BoardStatus boardStatus)
    {        
        rspStatusPanel.setFields(boardStatus);
        ethStatusPanel.setFields(boardStatus);
        mepStatusPanel.setFields(boardStatus);
        diagStatusPanel.setFields(boardStatus);
        blp0SyncStatusPanel.setFields(boardStatus.blp0Sync);
        blp1SyncStatusPanel.setFields(boardStatus.blp1Sync);
        blp2SyncStatusPanel.setFields(boardStatus.blp2Sync);
        blp3SyncStatusPanel.setFields(boardStatus.blp3Sync);
        blp0RcuStatusPanel.setFields(boardStatus.blp0Rcu);
        blp1RcuStatusPanel.setFields(boardStatus.blp1Rcu);
        blp2RcuStatusPanel.setFields(boardStatus.blp2Rcu);
        blp3RcuStatusPanel.setFields(boardStatus.blp3Rcu);
        rsuStatusPanel.setFields(boardStatus);
        blp0AdoStatusPanel.setFields(boardStatus.blp0AdcOffset);
        blp1AdoStatusPanel.setFields(boardStatus.blp1AdcOffset);
        blp2AdoStatusPanel.setFields(boardStatus.blp2AdcOffset);
        blp3AdoStatusPanel.setFields(boardStatus.blp3AdcOffset);
    }    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        rspStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.RSPStatusPanel();
        diagStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.DIAGStatusPanel();
        blp0SyncStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.SyncStatusPanel();
        blp1SyncStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.SyncStatusPanel();
        blp2SyncStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.SyncStatusPanel();
        blp3SyncStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.SyncStatusPanel();
        blp0RcuStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.RCUStatusPanel();
        blp1RcuStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.RCUStatusPanel();
        blp2RcuStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.RCUStatusPanel();
        blp3RcuStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.RCUStatusPanel();
        rsuStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.RSUStatusPanel();
        blp0AdoStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.ADOStatusPanel();
        blp1AdoStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.ADOStatusPanel();
        blp2AdoStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.ADOStatusPanel();
        blp3AdoStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.ADOStatusPanel();
        ethStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.ETHStatusPanel();
        mepStatusPanel = new nl.astron.lofar.java.gui.mac.jrsp.status.MEPStatusPanel();

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(rspStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(diagStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(ethStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(mepStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(blp1SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(blp0SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(blp2SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(blp3SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(blp1RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(blp1AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(layout.createSequentialGroup()
                        .add(blp0RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(blp0AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(layout.createSequentialGroup()
                        .add(blp2RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(blp2AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(layout.createSequentialGroup()
                        .add(blp3RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(blp3AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(rsuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(layout.createSequentialGroup()
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(rspStatusPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .add(diagStatusPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(ethStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(mepStatusPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .add(layout.createSequentialGroup()
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(blp0RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(blp0AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(layout.createSequentialGroup()
                                .add(blp1AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .add(blp2AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                            .add(layout.createSequentialGroup()
                                .add(blp1RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(blp2RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                            .add(blp3RcuStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(blp3AdoStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(rsuStatusPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .add(layout.createSequentialGroup()
                        .add(blp0SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(blp1SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(blp2SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(blp3SyncStatusPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(56, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents
       
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.java.gui.mac.jrsp.status.ADOStatusPanel blp0AdoStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.RCUStatusPanel blp0RcuStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.SyncStatusPanel blp0SyncStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.ADOStatusPanel blp1AdoStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.RCUStatusPanel blp1RcuStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.SyncStatusPanel blp1SyncStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.ADOStatusPanel blp2AdoStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.RCUStatusPanel blp2RcuStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.SyncStatusPanel blp2SyncStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.ADOStatusPanel blp3AdoStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.RCUStatusPanel blp3RcuStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.SyncStatusPanel blp3SyncStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.DIAGStatusPanel diagStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.ETHStatusPanel ethStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.MEPStatusPanel mepStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.RSPStatusPanel rspStatusPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.status.RSUStatusPanel rsuStatusPanel;
    // End of variables declaration//GEN-END:variables
}
