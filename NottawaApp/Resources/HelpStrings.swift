//
//  HelpStrings.swift
//  NottawaApp
//
//  Static string constants for the guided help tutorial.
//

import Foundation

enum HelpStrings {
    static let welcomeTitle = "Welcome to Nottawa!"
    static let welcomeSubtitle = "Thank you so much for being here.\n\nTo skip this tutorial, click the ? icon in the bottom right."

    static let startGuide = "Everything in Nottawa starts with a Source. A Source can be a live generated scene (\"effect\"), video file, webcam feed, and more."
    static let sourceDragInstruction = "To start, click and drag a Source from the sidebar over to the canvas."
    static let dropSourceInstruction = "Now drop the Source anywhere you like on the canvas."

    static let addEffectTitle = "Add an Effect"
    static let effectDragInstruction = "Grab any Effect you see in the sidebar and drag it to the canvas. If you're not sure, try Kaleidoscope or Pixelate."
    static let dropEffectTitle = "Drop the Effect anywhere you like on the canvas."

    static let connectSourceEffectTitle = "Let's connect our Source to the Effect."
    static let connectInstruction = "Click and drag from the filled pin on the right side of the Source\nto the hollow pin on the left side of the Effect."
    static let completeConnectionTitle = "Complete the connection"
    static let dragToEffectInstruction = "Drag to the hollow pin on your Effect."

    static let addSecondSourceTitle = "Adding another Source"
    static let addSecondSourceInstruction = "Add a second Source to the canvas."
    static let webcamVideoInstruction = "You can also use a Webcam or video from the Library."
    static let dropSecondSourceTitle = "Drop the Source on the canvas."

    static let addBlendEffectTitle = "Add a Blend Effect"
    static let dragBlendEffectInstruction = "You can find Blend under Favorites or the Mix column."
    static let dropBlendEffectTitle = "Drop the Blend Effect on the canvas."

    static let connectBlendEffectTitle = "Connect to the Blend Effect"
    static let combineNodesInstruction = "Let's combine all the nodes together. Drag from the output pin on this node to one of the input pins on Blend."
    static let completeBlendConnectionTitle = "Complete the connection"
    static let connectToBlendInstruction = "Connect with the input pin on Blend."

    static let addSecondBlendConnectionTitle = "Add a second connection to the second input on Blend."
    static let secondConnectionInstruction = "Drag from the output pin to the second input on Blend."
    static let completeSecondConnectionTitle = "Complete the connection"
    static let dropConnectionInstruction = "Drop the connection on the Blend."

    static let openShaderInfoTitle = "Open the Effect Settings"
    static let clickBlendInstruction = "Click inside the Blend node or its preview to open its Settings pane."

    static let changeParametersTitle = "Blend Settings"
    static let blendModeInstruction = "First, select a different Blend Mode to change how the images blend."
    static let alphaModeInstruction = "Now, use the slider for the Alpha parameter to change the opacity of the blend."
    static let otherNodesInstruction = "You can click on other nodes to change their settings, too."

    static let deleteBlend = "Now let's delete this Blend node.\n\nSince we've already got it selected, just press the Delete key on your keyboard to remove it."

    static let blendViaOutputsTitle = "Create a Blend"
    static let blendViaOutputsInstruction = "You can blend two nodes together by connecting their output pins.\n\nDrag from this output pin to the output pin on the other node."
    static let blendViaOutputsDrop = "Drop it on this output pin to create a Blend."

    static let actionButtonsTitle = "Common Actions"
    static let undoInstruction = "Undoes the last action"
    static let redoInstruction = "Redoes the last action"
    static let cameraViewInstruction = "Moves the camera so all the nodes are in view"
    static let helpToggleInstruction = "Turns this tutorial on/off"
    static let clearCanvasInstruction = "Clears the canvas, deleting all nodes and connections"
    static let scaleToFitInstruction = "Scales the canvas to fit the content"
    static let shareStrandInstruction = "Shares your strand with a link"

    static let shareStrandTitle = "Share Your Creation"
    static let shareStrandDescription = "You can share your node setup with others!"
    static let shareStrandStep1 = "Click the Share button (or right-click any node and select \"Share Strand...\")"
    static let shareStrandStep2 = "Add a title and description for your creation"
    static let shareStrandStep3 = "The preview image is captured automatically from your output"
    static let shareStrandStep4 = "Click Share to upload and get a link you can share anywhere!"
    static let shareStrandNote = "Others can open your strand directly in Nottawa from the link."

    static let completionTitle = "Thank you for being a part of Nottawa!"
    static let completionDescription = "I really appreciate you taking the time to try out this app. The goal of Nottawa is to make live video available for everyone, the best way for me to do that is to listen to you!"
    static let completionDescription2 = "I hope you can continue playing around and make something cool. Try using a Webcam, videos from the Library, or ones you've filmed yourself."
    static let completionDescription3 = "Once you're done, I'd love if you could share your thoughts. Use the menu option above, join the Discord, post in /r/nottawa_app, or just send me a message.\n\nWhat was cool? What was confusing? What was broken? What was missing? Everything helps!"
    static let completionDescription4 = "I love you all -\nJoe"
}
