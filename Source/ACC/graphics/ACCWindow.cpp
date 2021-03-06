ACCWindow::ACCWindow() :
    DocumentWindow (ProjectInfo::projectName, Colours::lightgrey, DocumentWindow::allButtons)
{
    setLookAndFeel (&lookAndFeel);

    setUsingNativeTitleBar (true);
    setResizable (true, false);

    constrainer.setMinimumSize (500, 500); //Limit minimum window size to something sensible
    setConstrainer (&constrainer);

    setContentOwned (new ACCMainComponent(), true);
    centreWithSize (getWidth(), getHeight());
    setVisible (true);
}

ACCWindow::~ACCWindow()
{
    setConstrainer (nullptr);
    setLookAndFeel (nullptr);
}

void ACCWindow::closeButtonPressed()
{
    JUCEApplication::quit();
}