/*
 * MepStatusPanel.java
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

package nl.astron.lofar.mac.apl.gui.jrsp.panels.status;

import nl.astron.lofar.mac.apl.gui.jrsp.BoardStatus;

/**
 * A panel that displays the MEP status data. This panel is used by the
 * StatusPanel.
 *
 * @author  balken
 */
public class MEPStatusPanel extends javax.swing.JPanel {
    
    /** Messages for error */
    String[] errorMessages = {"The MEP message was processed normally",
                              "Unknown message type",
                              "Illigal BLP address",
                              "Invalid PID",
                              "Register does not exist",
                              "Offset too large",
                              "Message is too large",
                              "Message CRC error on RSP ring interface",
                              "No ack message received from BLP"};
    
    /** 
     * Creates new form MEPStatusPanel.
     */
    public MEPStatusPanel() 
    {
        initComponents();
    }

    public void setFields(BoardStatus boardStatus)
    {
        if (boardStatus != null) {
            txtSeqNr.setText(Integer.toString(boardStatus.seqNr));
            txtError.setText(errorMessages[boardStatus.error]);
        } else {
            txtSeqNr.setText("");
            txtError.setText("");
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        lblSeqNr = new javax.swing.JLabel();
        lblError = new javax.swing.JLabel();
        txtSeqNr = new javax.swing.JTextField();
        txtError = new javax.swing.JTextField();

        setBorder(javax.swing.BorderFactory.createTitledBorder("MEP"));
        lblSeqNr.setText("Seq. Nr.");

        lblError.setText("Error");

        txtSeqNr.setEditable(false);
        txtSeqNr.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtError.setEditable(false);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                .addContainerGap(68, Short.MAX_VALUE)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(lblError)
                    .add(lblSeqNr))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                    .add(txtError)
                    .add(txtSeqNr, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 282, Short.MAX_VALUE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblSeqNr)
                    .add(txtSeqNr, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblError)
                    .add(txtError, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(17, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel lblError;
    private javax.swing.JLabel lblSeqNr;
    private javax.swing.JTextField txtError;
    private javax.swing.JTextField txtSeqNr;
    // End of variables declaration//GEN-END:variables
    
}
