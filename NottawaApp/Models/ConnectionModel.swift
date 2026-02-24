//
//  ConnectionModel.swift
//  NottawaApp
//
//  Represents a connection between two nodes in the graph.
//

import Foundation

struct ConnectionModel: Identifiable, Hashable {
    let id: String
    let startNodeId: String
    let endNodeId: String
    let outputSlot: Int
    let inputSlot: Int
    let connectionType: ConnectionInfo.ConnectionType

    func hash(into hasher: inout Hasher) {
        hasher.combine(id)
    }

    static func == (lhs: ConnectionModel, rhs: ConnectionModel) -> Bool {
        lhs.id == rhs.id
    }

    static func from(_ info: ConnectionInfo) -> ConnectionModel {
        ConnectionModel(
            id: info.id,
            startNodeId: info.startId,
            endNodeId: info.endId,
            outputSlot: info.outputSlot,
            inputSlot: info.inputSlot,
            connectionType: info.connectionType
        )
    }
}
